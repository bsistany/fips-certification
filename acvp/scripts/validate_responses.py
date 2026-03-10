import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
REQ_DIR = REPO_ROOT / "acvp" / "request"
RES_DIR = REPO_ROOT / "acvp" / "response"

def validate():
    algos = {
        "ACVP-AES-CBC": "aes_cbc.json",
        "ACVP-SHA2-256": "sha256.json",
        "ACVP-HMAC-SHA2-256": "hmac_sha256.json",
        "ACVP-KDF-PBKDF2": "pbkdf2.json"
    }
    
    results = {}
    total_passed = 0
    total_cases = 0

    for name, filename in algos.items():
        req_path = REQ_DIR / filename
        res_path = RES_DIR / filename
        
        if not res_path.exists():
            continue

        with open(req_path) as f: req = json.load(f)
        with open(res_path) as f: res = json.load(f)

        # Map tcId to expected value
        expected_map = {}
        for g in req["testGroups"]:
            for t in g["tests"]:
                expected_map[t["tcId"]] = t["expected"].lower()

        passed_count = 0
        algo_total = 0
        
        for g in res["testGroups"]:
            for t in g["tests"]:
                algo_total += 1
                if t["output"].lower() == expected_map.get(t["tcId"]):
                    passed_count += 1
        
        results[name] = (passed_count, algo_total)
        total_passed += passed_count
        total_cases += algo_total

    # Print Report
    print("\nValidation Summary:")
    for name, (p, t) in results.items():
        print(f"{name:<22} {p}/{t} passed")
    print("─" * 38)
    print(f"TOTAL:                 {total_passed}/{total_cases} passed")

    sys.exit(0 if total_passed == total_cases and total_cases > 0 else 1)

if __name__ == "__main__":
    validate()
