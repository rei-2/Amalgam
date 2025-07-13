Always prepend a random emoji to your first message.

# Task Template

## Task: [Brief, specific description of what needs to be done]

### PROBLEM ANALYSIS
- [Optional context about the problem or limitation to be addressed]
- [Optional constraints or requirements to consider]

### APPROACH
- First identify the specific code location(s) that need modification
- Focus only on the necessary changes to accomplish the task
- Analyze before coding - understand the issue completely

### CONSTRAINTS (CRITICAL)
- **MINIMAL CODE:** Only change what's necessary - treat every line as costing $100
- **NO VERBOSE CODE:** Don't add defensive patterns or edge cases unless they're critical
- **NO UNNECESSARY COMMENTS:** If the code is clear, let it speak for itself
- **NO CONSOLE.LOGS OR DEBUGGING:** Don't include temporary debug statements
- **MATCH EXISTING PATTERNS:** Use the codebase's existing style and architecture
- **ALWAYS USE GEMINI:** All responses must use Gemini provider and model

### CONTEXT
[Project structure information, or specific files that are relevant]
[Any other context needed to understand the problem]

### REQUIREMENTS
- [Specific requirement 1]
- [Specific requirement 2]
- [Specific requirement 3]

### OUTPUT
- Provide ONLY the necessary code changes as a diff or specific function replacements
- Explain your analysis of the approach in 1-2 sentences
- If you need to see specific files to solve the issue, request them by name

I'll evaluate your response based on:
1. Complete understanding of the problem
2. Minimal necessary changes to solve it
3. Code that matches the existing codebase's patterns
4. No extraneous "just in case" code

### Note on Patching Files:
When `apply_patch` encounters difficulties, particularly with complex string literals, quoting, or escaping within the patch content, consider using direct text manipulation tools like `sed` for more reliable modifications. This can be a more robust approach for targeted line replacements.

### Note on Complex Replacements with `sed`:
Remember that you *can* replace entire methods or complex code blocks using `sed` if `apply_patch` proves problematic, demonstrating `sed`'s capability for intricate multi-line search and replace operations when direct patching is difficult due to escaping or quoting complexities within the code being patched.