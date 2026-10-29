#include <assert.h>

int main(void)
{
    unsigned visits = 0;
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 4; ++col)
            ++visits;
    assert(visits == 8);
    return 0;
}
