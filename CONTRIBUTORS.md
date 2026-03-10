# Contributors

## Author

**Bahman Sistany** — architecture, design decisions, implementation, code
review, integration, testing, and documentation.

GitHub: [github.com/bsistany](https://github.com/bsistany)

---

## Development Tools

This project was developed with assistance from AI coding tools. All design
decisions, code review, integration, and testing were performed by the author.

| Tool | Role | Scope |
|---|---|---|
| Claude (Anthropic) | Architecture design, code generation, sprint planning | `acvp_runner.c`, `acvp/Makefile`, all documentation, sprint design |
| Gemini (Google) | Independent validation layer | `acvp/scripts/generate_vectors.py`, `acvp/scripts/validate_responses.py` |

Gemini was used deliberately as an independent author for the ACVP validation
scripts to establish a separation between the implementation under test and the
validation infrastructure. A bug in Gemini's `validate_responses.py` (incorrect
`tcId`-only lookup across multiple testGroups) was caught during integration
testing and corrected by the author.

---

## Third-Party Libraries

| Library | License | Use |
|---|---|---|
| [cJSON](https://github.com/DaveGamble/cJSON) | MIT | JSON parsing in `acvp_runner.c` |
| [cryptography](https://cryptography.io) (Python) | Apache 2.0 / BSD | Reference implementation in ACVP vector generation |

cJSON and the Python `cryptography` library are external dependencies of the
ACVP simulation pipeline only. They are not part of the cryptographic module
boundary and are not compiled into the core library.
