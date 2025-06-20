# Embedded System Final Project - Pet Feeder

## Security Setup (IMPORTANT)

⚠️ **Before building this project, you must create your secrets file:**

1. Copy `include/secrets.h.template` to `include/secrets.h`
2. Fill in your actual secret values in `include/secrets.h`
3. **NEVER commit `secrets.h` to git** - it's already in `.gitignore`

## Setup Instructions

1. Copy `secrets.ini.template` to `secrets.ini`
2. Fill in your actual secret values in `secrets.ini`
3. Run `python generate_secrets.py` to create `secrets.h`
4. Compile with `-DUSE_SECRETS` flag to use actual secrets

## Files NOT to commit:
- `include/secrets.h` - contains actual secret values (already ignored by git)
- `secrets.ini` - contains actual secret values
- `secrets.h` - auto-generated header with secrets