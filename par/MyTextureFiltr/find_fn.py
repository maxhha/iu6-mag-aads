m_i64H = 8559
"""
L = m_i64H - Win_cen * 2
K = L / a_iThreads
S(iThread) = K * iThread + Win_cen
iRow_start = S(iThread) - Win_cen
iRow_finish = S(iThread + 1) + Win_cen
"""


def magic_f(iThread, a_iThreads, Win_cen):
    L = m_i64H - Win_cen * 2
    K = L // a_iThreads

    if iThread < a_iThreads:
        return K * iThread + Win_cen
    return m_i64H - Win_cen


fallen = False

for Win_cen in range(1, 11):
    for a_iThreads in [1, 2, 4, 8]:
        returned = [
            magic_f(iThread=i, a_iThreads=a_iThreads, Win_cen=Win_cen)
            for i in range(a_iThreads + 1)
        ]
        deltas = [b - a for a, b in zip(returned, returned[1:])]

        if sum(deltas) != m_i64H - Win_cen * 2:
            print("Win_cen", Win_cen)
            print("Threads", a_iThreads)
            print("returned:", returned)
            print("deltas:", deltas)
            print("sum:", sum(deltas), "!=", m_i64H - Win_cen * 2)
            print()
            fallen = True
            break
    if fallen:
        break

print("Done")
