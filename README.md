# ECE 309 Project 1 - LLM Mini-Harness in C

## Overview

This project implements a small POSIX C harness that sits between terminal input and a mock language model. It demonstrates the four required ideas from Project 1: a terminal loop, bounded conversation context, tool execution, and AI-assisted testing/documentation.

## Requirements implemented

- Standard C in a POSIX environment.
- Terminal input collected with `fgets` inside a loop.
- `exit` performs a safe shutdown.
- Input containing the standalone word `hello` gets a hardcoded greeting.
- Other ordinary input is echoed by the mock model.
- The harness stores the last **5 user/assistant turns** and frees memory when turns leave the window or the program exits.
- `calc <expression>` calls a calculator tool implemented in C using a small recursive-descent parser. It supports `+`, `-`, `*`, `/`, unary `+`/`-`, decimal numbers, and parentheses.
- `history` prints the retained five-turn context so the state-management behavior can be tested deterministically.
- `test.sh` compiles with strict warnings, runs deterministic behavior checks

## Build

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror harness.c -o harness
```

## Run

```bash
./harness
```

Example session:

```text
ECE 309 LLM Mini-Harness
Type a message, 'calc <expression>', 'history', or 'exit'.
You> hello
Assistant> Hello! This is the mock model greeting.
You> calc 2 + 3 * 4
Assistant> Tool result: 14
You> hello again
Assistant> Hello! This is the mock model greeting.
You> exit
Goodbye.
```

## Automated testing

Run:

```bash
bash test.sh
```

The script checks the core interaction, calculator tool, `exit`, five-turn rolling history, and basic memory safety.

## Project files

- `harness.c` - single-file C implementation.
- `test.sh` - automated Bash test script.
- `README.md` - build, run, and design documentation.
- `vibe_coding_log.md` - prompts, design constraints, and iteration notes.
- `github.txt` - placeholder for the final GitHub repository URL.
