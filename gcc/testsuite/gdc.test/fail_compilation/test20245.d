/*
REQUIRED_ARGS: -preview=dip1000
TEST_OUTPUT:
---
fail_compilation/test20245.d(20): Error: reference to local variable `x` assigned to non-scope parameter `ptr`
fail_compilation/test20245.d(21): Error: copying `&x` into allocated memory escapes a reference to parameter `x`
fail_compilation/test20245.d(22): Error: scope variable `a` may not be returned
fail_compilation/test20245.d(26): Error: cannot take address of `scope` variable `x` since `scope` applies to first indirection only
fail_compilation/test20245.d(32): Error: reference to local variable `x` assigned to non-scope parameter `ptr`
fail_compilation/test20245.d(33): Error: copying `&x` into allocated memory escapes a reference to parameter `x`
fail_compilation/test20245.d(49): Error: reference to local variable `price` assigned to non-scope `this.minPrice`
fail_compilation/test20245.d(68): Error: reference to local variable `this` assigned to non-scope parameter `msg`
fail_compilation/test20245.d(88): Error: reference to local variable `this` assigned to non-scope parameter `content`
---
*/

// https://issues.dlang.org/show_bug.cgi?id=20245
@safe int* foo(ref int x) {
    int* a = &x;
    escape(&x);
    auto b = [&x];
    return a;
}

@safe int** foo(ref scope int* x) {
    int** a = &x;
    return a;
}

@safe int* foo(return ref int x) {
    int* a = &x;
    escape(&x);
    auto b = [&x];
    return a;
}

int* gPtr;
@safe void escape(int* ptr)
{
    gPtr = ptr;
}

// https://issues.dlang.org/show_bug.cgi?id=21212
class MinPointerRecorder
{
    int* minPrice;
    void update(ref int price) @safe
    {
        minPrice = &price; // Should not compile.
    }
}

void main() @safe
{
    auto r = new MinPointerRecorder;
    () { int mp = 42; r.update(mp); } ();
    () { ulong[1000] stomp = 13; } ();
    auto x = *r.minPrice; // "13"
}

// https://issues.dlang.org/show_bug.cgi?id=22782
struct DontDoThis
{
    immutable char[12] content;
    @safe this(char ch)
    {
        content[] = ch;
        throw new Exception(content[]);
    }
}

void main1() @safe
{
    DontDoThis('a');
}

// https://issues.dlang.org/show_bug.cgi?id=22783
const(char)* charPtr;

// argument is not, or should not be scope
auto listUp(const(char)* content) {charPtr = content;}

struct DontDoThis2
{
    char content;
    @safe escape()
    {
        listUp(&content);
    }
}
