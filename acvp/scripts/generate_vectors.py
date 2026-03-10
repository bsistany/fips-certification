import json
import os
import random
import string
from pathlib import Path
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

# Paths resolved relative to repo root
REPO_ROOT = Path(__file__).resolve().parents[2]
REQUEST_DIR = REPO_ROOT / "acvp" / "request"
REQUEST_DIR.mkdir(parents=True, exist_ok=True)

def write_json(filename, data):
    with open(REQUEST_DIR / filename, 'w') as f:
        json.dump(data, f, indent=2)
        f.write('\n')

def generate_aes_cbc():
    groups = []
    for tg_id, direction in [(1, "encrypt"), (2, "decrypt")]:
        tests = []
        for tc_id in range(1, 5):
            key = os.urandom(32)
            iv = os.urandom(16)
            # Exact multiple of 16 bytes
            pt = os.urandom(random.randint(1, 8) * 16)
            
            cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
            if direction == "encrypt":
                encryptor = cipher.encryptor()
                ct = encryptor.update(pt) + encryptor.finalize()
                tests.append({
                    "tcId": tc_id, "key": key.hex(), "iv": iv.hex(),
                    "pt": pt.hex(), "expected": ct.hex()
                })
            else:
                encryptor = cipher.encryptor()
                ct = encryptor.update(pt) + encryptor.finalize()
                decryptor = cipher.decryptor()
                expected_pt = decryptor.update(ct) + decryptor.finalize()
                tests.append({
                    "tcId": tc_id, "key": key.hex(), "iv": iv.hex(),
                    "ct": ct.hex(), "expected": expected_pt.hex()
                })
        groups.append({"tgId": tg_id, "direction": direction, "keyLen": 256, "tests": tests})
    
    write_json("aes_cbc.json", {"vsId": 1, "algorithm": "ACVP-AES-CBC", "revision": "1.0", "testGroups": groups})

def generate_sha256():
    tests = []
    # Categories: Empty, Short (1-63), Block (64)
    lengths = [0] + [random.randint(1, 63) for _ in range(5)] + [64] * 5
    for i, length in enumerate(lengths, 1):
        msg = os.urandom(length)
        digest = hashes.Hash(hashes.SHA256())
        digest.update(msg)
        expected = digest.finalize()
        tests.append({"tcId": i, "len": length * 8, "msg": msg.hex(), "expected": expected.hex()})
    
    write_json("sha256.json", {
        "vsId": 2, "algorithm": "ACVP-SHA2-256", "revision": "1.0", 
        "testGroups": [{"tgId": 1, "testType": "AFT", "tests": tests}]
    })

def generate_hmac():
    groups = []
    configs = [(1, 16, 16), (2, 32, 32), (3, 64, 32)]
    for tg_id, k_len, m_len in configs:
        tests = []
        for tc_id in range(1, 5):
            key = os.urandom(k_len)
            msg = os.urandom(random.randint(16, 128))
            h = hmac.HMAC(key, hashes.SHA256())
            h.update(msg)
            expected = h.finalize()[:m_len]
            tests.append({"tcId": tc_id, "key": key.hex(), "msg": msg.hex(), "expected": expected.hex()})
        groups.append({"tgId": tg_id, "keyLen": k_len * 8, "macLen": m_len * 8, "tests": tests})
    
    write_json("hmac_sha256.json", {"vsId": 3, "algorithm": "ACVP-HMAC-SHA2-256", "revision": "1.0", "testGroups": groups})

def generate_pbkdf2():
    tests = []
    # Password/Salt are ASCII strings
    charset = string.ascii_letters + string.digits
    key_lens = [16, 32, 64] * 4
    random.shuffle(key_lens)
    
    for i, k_len in enumerate(key_lens, 1):
        pwd = ''.join(random.choice(charset) for _ in range(random.randint(8, 32)))
        salt = ''.join(random.choice(charset) for _ in range(random.randint(8, 32)))
        kdf = PBKDF2HMAC(algorithm=hashes.SHA256(), length=k_len, salt=salt.encode(), iterations=10000)
        expected = kdf.derive(pwd.encode())
        tests.append({
            "tcId": i, "password": pwd, "salt": salt, "iterationCount": 10000,
            "keyLen": k_len * 8, "hashAlg": "SHA2-256", "expected": expected.hex()
        })
    
    write_json("pbkdf2.json", {
        "vsId": 4, "algorithm": "ACVP-KDF-PBKDF2", "revision": "1.0",
        "testGroups": [{"tgId": 1, "tests": tests}]
    })

if __name__ == "__main__":
    print("Generating ACVP Vectors...")
    generate_aes_cbc()
    generate_sha256()
    generate_hmac()
    generate_pbkdf2()
    print("Done. Request files written to acvp/request/")
