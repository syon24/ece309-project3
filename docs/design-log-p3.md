# Project 3 Design Log

**P2 code used:** The supplied Project 2 Conversation and SentinelScanner files, copied unchanged. The instructor reference was not used.

**Tools used:** OpenAI Codex assisted with helping me understand thhe specification, It also assisted me when I got stuck on the three P3 components as well as tests.  Local checks ran on Windows; the required Linux ASan/UBSan check was succesful.

## Hash table

The registry uses separate chaining to handle collisions. Each bucket contains a vector of entries, with each entry storing a name and a `unique_ptr` to its tool. If two names hash to the same bucket, the new entry is simply added to that bucket. When looking up a tool, the registry hashes its name and searches through the entries in that specific bucket. This approach works well with the given interface and avoids the need for empty-slot markers or probing through the table.

The hash function follows the byte-by-byte XOR and multiplication process shown in Appendix A, using the same constants. Each character contributes to the final hash value, and the order of the characters makes a difference. The function returns the full hash value, while the bucket index is calculated separately using modulo. The tool's name is also stored when it is first registered. This way, the registry can rehash entries without having to call the tool's virtual name function again.

The table begins with 16 buckets and uses a load-factor threshold of 1.0. This makes sense for separate chaining since multiple entries can exist in the same bucket. If adding a new entry would push the load factor above that limit, the number of buckets is doubled. The existing entries are then moved into their new buckets based on their hash values. Before any resizing happens, the registry checks for null pointers and duplicate names, rejecting them if found.

The 10,000-tool test produced these measurements on the local 64-bit build:

| Measurement | Appendix A hash | Weak first-character-plus-length hash |
|---|---:|---:|
| Tools | 10,000 | 10,000 |
| Buckets | 16,384 | 16,384 |
| Load factor | 0.61035156 | 0.61035156 |
| Longest chain | 6 | 9,000 |

The weak hash function ignores most of the characters in a name. Since all the four-digit suffixes have the same length and start with the same character, all 9,000 names end up in the same bucket. The implemented hash function does a better job of spreading these names across different buckets, which helps reduce collisions. The test also makes sure that every name can still be found after being inserted.

Resizing the table takes O(n) time because all existing entries need to be moved. However, since the bucket count doubles each time, resizing happens less frequently as the table grows. For example, resizing at 16, 32, 64, and so on creates a geometric sum that grows in proportion to the total number of entries inserted. When this cost is spread across all insertions, the average resizing cost per insertion is constant. As long as the hash function distributes names evenly and the load factor stays within its limit, adding a new entry has an amortized time complexity of O(1), assuming name length is bounded.

## Pointer stability

Moving a unique_ptr transfers ownership without moving the object it points to. Both bucket growth and table rehashing move entries, but the Tool objects stay at their original heap addresses. A pointer returned by find stays valid until the registry is destroyed. The scale test saves the first pointer and compares it after all 10,000 insertions. A separate destructor-count test checks duplicate rejection and cleanup.

## Calculator trace
For the expression `2 * (3 + 4) - 5`, the stacks are shown from bottom to top. First, reading 2 adds `[2]` to the values stack. Then, reading * adds `[*]` to the ops stack. When the opening parenthesis is found, the ops stack becomes `[*, (]`. Next, reading 3 updates the values stack to `[2, 3]`, and reading + changes the ops stack to `[*, (, +]`. After reading 4, the values stack contains `[2, 3, 4]`.

When the closing parenthesis is reached, the + operator is evaluated by popping 4 as the right operand and 3 as the left operand. The result is 7, so the values stack becomes `[2, 7]`. The opening parenthesis is then removed from the ops stack. When the - operator is read, multiplication is evaluated first because it has higher precedence. This gives 14, leaving the values stack as `[14]` and the ops stack as `[-]`. Reading 5 adds it to the values stack, making `[14, 5]`. Finally, the remaining subtraction is evaluated, giving the final result of 9.

Operators with equal precedence are also evaluated before adding the next operator. This makes sure subtraction and division are processed from left to right. The implementation also checks that the stacks have enough elements before accessing or popping values to avoid errors. Loops are used to handle long expressions and deeply nested parentheses without needing recursion.
## Errors as values

A forbidden path or division by zero can happen during ordinary tool use. Returning false with an explanation lets the provided dispatcher send an error result to the model, which can make another request. Exceptions are better suited to programming mistakes, such as using the wrong JSON accessor. The dispatcher's exception handler is a last safeguard.

## What could change

The registry duplicates each tool's name in its entry. That simplifies lookup and rehashing, but uses extra memory. With a redesigned Tool interface, a stable name reference could avoid that copy. For this project, keeping the required interface and a clear ownership model is more useful than saving those bytes.
