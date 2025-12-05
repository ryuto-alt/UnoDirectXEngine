# Project Context
- **Core**: DirectX 12 Ultimate, HLSL (Shader Model 6.6+), C++20/23.
- **Goal**: High-performance rendering engine (UnoEngine).
- **Philosophy**: Explicit synchronization. No "magic" fixes. Modern C++ ONLY.

# Critical Directives (Strict Enforcement)
1. **Modern C++ Standard (C++20/23)**:
   - **FORBIDDEN**: `new`/`delete`, `NULL`, C-style arrays (`int arr[]`), `typedef`, raw pointers (except non-owning), `try-catch` (Exceptions).
   - **MANDATORY**: `std::unique_ptr`, `nullptr`, `std::span`, `std::array`, `using`, `std::format`, `std::filesystem`.
   - **Modules**: Prefer C++20 Modules logic if project structure allows, otherwise standard headers.

2. **Error Handling & Naming**:
   - **Result Handling**: Strict `HRESULT` checking for DX12. Use `std::expected` (C++23) or `std::optional` for return values. Do NOT use exceptions.
   - **Naming Convention**:
     - Types/Classes/Functions: `PascalCase`
     - Variables/Parameters: `camelCase`
     - Private Members: `m_camelCase`
     - Statics: `s_camelCase`

3. **Comment Style (Minimalist)**:
   - **Do NOT** write essay-like comments explaining obvious code (e.g., `// Increment i`).
   - **Do NOT** comment out old code blocks. Delete them. We use Git.
   - Write brief, "Why-focused" comments only for complex logic (e.g., sync barriers, math hacks).
     - *Bad*: `// Create the descriptor heap`
     - *Good*: `// Shader-visible for bindless rendering`

4. **Interaction Protocol (Question-First)**:
   - **STOP & ASK**: Before implementing ANY feature, use the Question/Task UI to interview the user.
   - **Confirm Specs**: Ask 3-5 clarification questions about requirements, edge cases, and constraints.
     - *Example*: "Should this buffer be CPU-writable or GPU-local?", "Is this for the main render pass or compute?"
   - **Plan Approval**: Output a step-by-step implementation plan and wait for explicit "OK" before writing code.

# Workflow Constraints
1. **No Auto-Build**: User handles compilation. Wait for feedback.
2. **No Ghost Files**: Do not create `.md` files. Use `serenaMCP` for notes.
3. **Tool Usage**:
   - **Research**: ALWAYS use `braveMCP` for API specs. No guessing.
   - **Memory**: Use `serenaMCP` to record architectural decisions.

# Architecture Directives (DX12)
- **Heaps**: Ring-buffer allocation for dynamic descriptors.
- **Barriers**: Explicit state transitions. Verify `Present` state.
- **Alignment**: `CBV` = 256 bytes. Structs must match HLSL packing rules (`!hlsl` to check).

# Custom Commands
- `!research`: `braveMCP` search.
- `!memo`: `serenaMCP` write.
- `!hlsl`: Check C++/HLSL alignment.
- `!plan`: Start the "Question-First" interview process for a new task.