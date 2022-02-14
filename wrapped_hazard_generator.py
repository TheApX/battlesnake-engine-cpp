x = 0
y = 0

def printPos():
    print('{%d, %d},' % (x, y))

print('static constexpr std::initializer_list<Point> kHazardList = {')
printPos()
size = 1
while size < 50:
    # Up
    for i in range(size):
        y = y + 1
        printPos()
    # Right
    for i in range(size):
        x = x + 1
        printPos()
    size = size + 1
    # Down
    for i in range(size):
        y = y - 1
        printPos()
    # Left
    for i in range(size):
        x = x - 1
        printPos()
    size = size + 1
print('};')
