a = [int(i) for i in input().split()]
b = [int(i) for i in input().split()]

for i in range(1000_000):
    ai, *a = a
    bi, *b = b
    # print(a, b)

    if ai > bi or (ai == 0 and bi == 9):
        a.append(ai)
        a.append(bi)
    else:
        b.append(ai)
        b.append(bi)

    if len(a) == 0:
        print(f"second {i+1}")
        break
    if len(b) == 0:
        print(f"first {i+1}")
        break
else:
    print("botva")
