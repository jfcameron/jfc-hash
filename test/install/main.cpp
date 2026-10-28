// © Joseph Cameron - MIT License

#include <jfc/hash.h>

int main() {
    jfc::hash::splitmix64 random(1234567);

    const bool known = random() == 6457827717110365317ull
        && jfc::hash::fnv1a64{}.add("a").value() == 0xaf63dc4c8601ec8cull;

    return known ? 0 : 1;
}
