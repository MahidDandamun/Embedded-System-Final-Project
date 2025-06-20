# ...existing code...

## Setup Instructions

1. Copy `secrets.ini.template` to `secrets.ini`
2. Fill in your actual secret values in `secrets.ini`
3. Run `python generate_secrets.py` to create `secrets.h`
4. Compile with `-DUSE_SECRETS` flag to use actual secrets

## Files NOT to commit:
- `secrets.ini` - contains actual secret values
- `secrets.h` - auto-generated header with secrets

# ...existing code...