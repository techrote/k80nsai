#!/usr/bin/env python3
"""Validate the task graph and local links; --live also checks public issue mapping."""
from __future__ import annotations
import argparse
import json
import re
import sys
from pathlib import Path
from urllib.request import Request, urlopen


def check_graph(data: dict) -> list[str]:
    errors: list[str] = []
    tasks = data['tasks']
    by_id = {t['id']: t for t in tasks}
    if len(by_id) != len(tasks):
        errors.append('Duplicate task ID')
    numbers = [t['issue'] for t in tasks] + [data['master_issue']]
    if len(numbers) != len(set(numbers)):
        errors.append('Duplicate issue number')
    state: dict[str, int] = {}
    def visit(key: str) -> None:
        if state.get(key) == 1:
            errors.append(f'Dependency cycle at {key}')
            return
        if state.get(key) == 2:
            return
        state[key] = 1
        task = by_id[key]
        deps = set(task['start_after'] + task['close_after'])
        for field in ('evidence_before_start', 'evidence_before_close'):
            for gate in task.get(field, []):
                if not gate.get('artifact'):
                    errors.append(f'{key}: unnamed evidence artifact')
                deps.add(gate.get('task', ''))
        for dep in deps:
            if dep not in by_id:
                errors.append(f'{key}: unknown dependency {dep}')
                continue
            if not task.get('deferred') and by_id[dep].get('deferred'):
                errors.append(f'{key}: core task depends on deferred {dep}')
            visit(dep)
        if task.get('deferred') and not task.get('owner_approval_required'):
            errors.append(f'{key}: deferred extension lacks owner approval')
        state[key] = 2
    for key in by_id:
        visit(key)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--live', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    data = json.loads((root / 'docs/workflow.json').read_text(encoding='utf-8'))
    errors = check_graph(data)
    docs = [root / name for name in ('README.md', 'OVERVIEW.md', 'AGENTS.md')]
    docs += sorted((root / 'docs').rglob('*.md'))
    for doc in docs:
        if not doc.is_file():
            errors.append(f'Missing document {doc.relative_to(root)}')
            continue
        for link in re.findall(r'\]\(([^)]+)\)', doc.read_text(encoding='utf-8')):
            if '://' in link or link.startswith(('#', 'mailto:')):
                continue
            path = link.split('#', 1)[0]
            if path and not (doc.parent / path).exists():
                errors.append(f'{doc.relative_to(root)}: missing local link {path}')
    atlas = (root / 'OVERVIEW.md').read_text(encoding='utf-8')
    for task in data['tasks']:
        if f"https://github.com/{data['repository']}/issues/{task['issue']}" not in atlas:
            errors.append(f"Atlas missing issue link for {task['id']}")
    if args.live:
        try:
            url = f"https://api.github.com/repos/{data['repository']}/issues?state=all&per_page=100"
            req = Request(url, headers={'User-Agent': 'k80nsai-workflow-check'})
            with urlopen(req, timeout=30) as response:
                items = json.load(response)
            issues = {item['number']: item for item in items if 'pull_request' not in item}
            for task in data['tasks']:
                item = issues.get(task['issue'])
                if item is None or f"[{task['id']}]" not in item['title']:
                    errors.append(f"Live issue mapping mismatch for {task['id']}")
            if data['master_issue'] not in issues:
                errors.append('Missing live master issue')
        except Exception as exc:
            errors.append(f'Live check unavailable/failed: {exc}')
    if errors:
        print('\n'.join('ERROR: ' + error for error in errors), file=sys.stderr)
        return 1
    core = sum(not t.get('deferred', False) for t in data['tasks'])
    print(f"PASS: {core} core tasks, {len(data['tasks'])-core} deferred extensions; graph and links valid")
    print('This does not validate CUDA, model execution, issue completion or server-enforced dependencies.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
