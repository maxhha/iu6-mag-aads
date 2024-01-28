#include <iostream>
#include <set>
#include <vector>

using namespace std;

int main()
{
    int n, w;
    cin >> n >> w;

    vector<int> p(n);
    vector<pair<int, int>> weight(w + 1, {0, 0});
    for (int i = 0; i < n; ++i)
    {
        cin >> p[i];
    }
    if (w == 0)
    {
        cout << "0\n0\n";
        return 0;
    }
    weight[0].first = 1;
    weight[0].second = -1;
    for (int i = 0; i < n; ++i)
    {
        for (int j = w; j > -1; --j)
        {
            if (weight[j].first == 1)
            {
                if (j + p[i] < w + 1)
                {
                    if (weight[j + p[i]].first == 1)
                        continue;
                    weight[j + p[i]].first = 1;
                    weight[j + p[i]].second = j;
                }
            }
        }
    }
    multiset<int> result;
    for (int j = w; j > -1; --j)
    {
        if (weight[j].first == 1)
        {
            cout << j << "\n";
            int currW = j;
            int prevW = j + 1;
            while (currW != -1)
            {
                prevW = currW;
                currW = weight[currW].second;
                if (currW == -1)
                    break;
                result.insert(prevW - currW);
            }
            break;
        }
    }
    cout << result.size() << "\n";
    for (auto elem : result)
    {
        cout << elem << " ";
    }
    return 0;
}
