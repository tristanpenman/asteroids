# Conventions

These conventions apply to the project-owned C code in `src`. When modifying older code, apply them to the code being changed when doing so is low risk.

## Core Guidelines

### Code Style

- Use C and keep implementations in `.c` files with public declarations in `.h` files.
- Indent with four spaces. Do not use tabs.
- Put a function's opening brace on the next line.
- Declare and define functions that take no arguments with an explicit `void` parameter list, such as `void module_reset(void)`.
- Put the opening brace on the same line for `if`, `else`, `for`, `while`, and `switch` statements.
- Put the opening brace for a multi-line `struct` or `enum` definition on the next line. Follow the surrounding file for compact definitions that already use a same-line brace.
- Always use braces around control-flow bodies, including single-statement bodies.
- Put one space after control-flow keywords and around binary operators.
- Bind pointer stars to the variable or parameter name: `struct item *item` and `const char *text`.
- Separate logical sections within a function with blank lines. Keep related module-level declarations together.
- For a declaration or call that does not fit comfortably on one line, break at parameter boundaries and indent continuation lines by four spaces. Keep a group of closely related parameters together when that makes the signature easier to scan.
- Follow the existing file's line length and line endings. Wrap long calls and diagnostics rather than reformatting unrelated code.

```c
static bool update_item(
    struct item *item, const struct options *options,
    float amount)
{
    if (!item->enabled) {
        return false;
    }

    item->value = options->value;
    item_apply(item, amount);
    return true;
}
```

### Naming

- Files and project-defined functions use `snake_case`. Public functions are normally prefixed with their module name, such as `module_init`, `module_update`, and `module_cleanup`.
- Local variables, parameters, structure fields, and module-level variables use `snake_case`.
- Structure and enumeration tags use lower-case `snake_case` and are written explicitly at use sites: `struct item_data` and `enum item_state`.
- Function-pointer typedefs use a `_fn_t` suffix, as in `update_fn_t`.
- Preprocessor constants and macros use `UPPER_SNAKE_CASE`. Parenthesize macro parameters and the complete replacement expression when a macro performs a calculation.
- Enumeration values use an upper-case prefix associated with their enum or module, such as `ITEM_READY`, `ITEM_ACTIVE`, and `ITEM_COMPLETE`.
- Short names such as `p`, `v`, `f`, and `n` are acceptable for conventional values in small, focused functions. Use descriptive names when a value's role is not immediately clear.
- Preserve names prescribed by a library API or platform callback signature.

### Headers and includes

- Give every project header an include guard. Guard name should be based on `<PROJECT>_<MODULE...>_<FILE>_H`. E.g. `MYLIB_COMMON_STRINGS_H` for a project called MyLib, and a header at `common/strings.h`.
- Include what a file directly uses. Use a forward declaration for a structure that appears only through a pointer; include its defining header when the complete type is required.
- Group includes in this order, separated where useful by a blank line:
  1. Standard-library headers.
  2. Platform or library headers, including platform-conditional headers.
  3. Project headers.
- Keep project headers in alphabetical order where practical.
- Keep platform-specific includes behind the relevant preprocessor condition.
- Public headers should expose only the dependencies required by their public declarations.
- Put declarations in the header for the module's public interface. Keep helper functions and implementation-only state in the `.c` file and declare them `static`.

### Types, ownership, and interfaces

- Prefer plain tagged `struct` and `enum` types over typedef aliases for data models. Group related state into structures such as `struct item` and `struct options`.
- Pass small value objects by value where the existing API does so. Pass larger or mutable objects by pointer.
- Add `const` to pointer parameters and module-level data that are not modified.
- Use `bool` and the `true` and `false` values supplied through `types.h` for boolean state and results.
- Use `NULL` for null pointers and named negative constants for invalid handles or error results.
- Use `size_t` for byte counts and results returned by the C library. Use fixed-width unsigned types for stored data and APIs whose width is significant.
- Make ownership explicit in the implementation. Check allocation and resource acquisition results, release resources in the reverse lifecycle operation, and reset retained pointers or state after cleanup.
- Represent callbacks with function-pointer types or explicit function-pointer parameters. Keep callback implementations small and give them module-specific names.
- Use file-scope state only when it belongs to a module's lifecycle. Mark it `static` unless it is intentionally part of another module's interface.

### Control flow and errors

- Return early for invalid input, unavailable resources, and completed state transitions when that keeps the main path clear.
- Use `bool` for success/failure APIs and named integer error codes when callers need to distinguish failure modes.
- Check C library and external API return values where failure can be handled or reported. Write user-facing failures to `stderr`; use the project's debug logging and assertion helpers for development diagnostics and invariants.
- Keep paired acquisition and release operations visible and balanced. Clean up partially acquired resources on failure paths.
- Use `switch` for event and state dispatch, with one `case` per logical input and an explicit `default` when unmatched values require handling.

### Comments and documentation

- Explain intent, constraints, platform differences, units, and non-obvious behavior rather than restating the code.
- Use short `//` comments for local implementation notes. Use `/* ... */` only where it is clearer in surrounding code.
- Public APIs with non-obvious contracts may use Doxygen-style `/** ... */` comments with `@param` and `@return` entries.
- Large implementation files may use the established banner comments to mark sections such as helper functions and the public interface.
- Keep TODO comments specific enough to identify the missing work.

### Compatibility

- Use `f` suffixes for floating-point literals used in `float` calculations. Cast deliberately at API or arithmetic boundaries where implicit conversion would obscure the intended type.

### Example

An example header `shared/counter.h` for a project called `MyLib`:

```c
#ifndef MYLIB_SHARED_COUNTER_H
#define MYLIB_SHARED_COUNTER_H

#include "types.h"

struct counter
{
    int value;
    bool enabled;
};

bool counter_update(struct counter *counter, int amount);

#endif
```

A corresponding source file `shared/counter.c`:

```c
#include <stddef.h>

#include "counter.h"

static bool amount_is_valid(int amount)
{
    return amount > 0;
}

bool counter_update(struct counter *counter, int amount)
{
    if (NULL == counter || !counter->enabled || !amount_is_valid(amount)) {
        return false;
    }

    counter->value += amount;
    return true;
}
```

## Integrations

## N64 SDK

- Preserve SDK-defined names and types, including their mixed-case spelling. Use project conventions for identifiers that the project owns.
- Use SDK integer types (e.g. `s32`, `u32`) where an API or binary layout requires them, and apply the required alignment to data read by the RSP or RDP.
- Convert virtual addresses to physical addresses at SDK boundaries. Keep display-list buffers bounded and check their size before submission.
- Preserve generated asset names and layout unless the generator is updated with them.
