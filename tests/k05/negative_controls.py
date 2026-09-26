"""Prove that rejection and diagnostic paths really fail at process level."""
import subprocess
import sys

binary = sys.argv[1]
rejects = """storage tail zero-columns zero-rows block-count stride group-stride
offset extent-overflow row-bytes dtype null-storage row-index group-index
basis-count activation-count null-activation nan inf minus-inf half-inf half-nan
threshold threshold-nan threshold-inf threshold-above depth depth-four
energy-product energy-sum energy-underflow alpha-overflow dot-product dot-sum
bad-basis bad-alpha computed-depth negative-alpha""".split()
count = 0
for option, names in (("--reject", rejects), ("--inject", ["decode", "mask", "basis", "adaptive"])):
    for name in names:
        result = subprocess.run([binary, option, name], text=True, capture_output=True, check=False)
        output = result.stdout + result.stderr
        required = ["seed=4927541"]
        required += ["REJECT", "negative=" + name] if option == "--reject" else [
            "FAIL", "case=injected-" + name, "row=0", "group=0", "half_bits=",
            "block_bytes=", "x[0]=", "requested_depth=3", "tau1=0", "expected=", "actual=",
        ]
        if result.returncode != 1 or any(token not in output for token in required):
            print(f"FAIL control {option} {name}: exit={result.returncode}\n{output}")
            sys.exit(1)
        count += 1
print(f"PASS negative_controls={count} expected_process_exit=1 diagnostic_tokens_verified")