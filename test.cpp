#include <bits/stdc++.h>

using namespace std;
using ll = long long;
ll INF = 1 << 60;

struct Edge {
    int to;
    ll interval;
    ll deadline;
    int required;
};

struct Item {
    int v;
    ll start;
    ll close;
    int cost;
};

struct State {
    int v;
    int mask;
    ll time;
    ll lo;
    ll hi;
};

struct Later {
    bool operator()(const State& a, State& b) const {
        return a.time > b.time;
    }
};

bool dominates(const State& a,const State& b) {
    return a.time <= b.time && a.lo <= b.lo && a.hi >= b.hi;
}

void add_label(const State& c, ll T_lim,
               vector<vector<vector<State>>>& labels,
               priority_queue<State, vector<State>, Later>& pq) {
    if (c.lo > c.hi || c.lo + c.time > T_lim) {
        return;
    }

    auto& cur_st = labels[c.v][c.mask];
    for (const State& o : cur_st) {
        if (dominates(o, c)) return;
    }
    size_t kept = 0;
    for (size_t i = 0; i < cur_st.size(); ++i) {
        if (!dominates(c, cur_st[i])) {
            cur_st[kept] = cur_st[i];
            ++kept;
        }
    }
    cur_st.resize(kept);
    cur_st.push_back(c);
    pq.push(c);
}

void solve(int n, int item_cnt, int start, ll T_lim,
           vector<vector<Edge>>& graph,
           vector<Item>& items,
           vector<vector<int>>& items_at,
           vector<vector<vector<State>>>& labels) {
    labels.assign(n, vector<vector<State>>(1 << item_cnt));
    priority_queue<State, vector<State>, Later> pq;

    add_label({start, 0, 0, -INF, INF}, T_lim, labels, pq);

    while (!pq.empty()) {
        State cur = pq.top();
        pq.pop();

        bool check_v = false;
        for (State& o : labels[cur.v][cur.mask]) {
            if (o.time == cur.time && o.lo == cur.lo && o.hi == cur.hi) {
                check_v = true;
                break;
            }
        }
        if (!check_v) continue;

        for (int i : items_at[cur.v]) {
            if (cur.mask >> i & 1) continue;

            ll t = max(cur.time, items[i].start - cur.hi);
            ll lo = max(cur.lo, items[i].start - t);
            ll hi = min(cur.hi, items[i].close - t);
            add_label({cur.v, cur.mask | (1 << i), t, lo, hi}, T_lim, labels, pq);
        }

        for (Edge& e : graph[cur.v]) {
            if ((cur.mask & e.required) != e.required) continue;

            ll nt = cur.time + e.interval;
            ll hi = min(cur.hi, e.deadline - nt);
            add_label({e.to, cur.mask, nt, cur.lo, hi}, T_lim, labels, pq);
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, start;
    cin >> n >> m >> start;

    vector<vector<Edge>> graph(n);
    for (int i = 0; i < m; ++i) {
        int u, v, q;
        ll t, deadline;
        cin >> u >> v >> t >> deadline >> q;
        int required = 0;
        for (int j = 0; j < q; ++j) {
            int id;
            cin >> id;
            required |= (1 << id);
        }
        if (deadline < 0) deadline = INF;
        graph[u].push_back({v, t, deadline, required});
    }

    int item_cnt;
    cin >> item_cnt;
    vector<vector<int>> items_at(n);
    vector<Item> items(item_cnt);
    for (int i = 0; i < item_cnt; ++i) {
        int v, cost;
        ll open, close;
        cin >> v >> open >> close >> cost;
        if (close < 0) open = -INF, close = INF;
        items_at[v].push_back(i);
        items[i] = {v, open, close, cost};
    }

    int ex;
    ll T_lim;
    double T_cost;
    cin >> T_lim >> T_cost >> ex;
    vector<int> exits(n, 0);
    for (int i = 0; i < ex; ++i) {
        int v;
        cin >> v;
        exits[v] = 1;
    }

    vector<int> value(1 << item_cnt, 0);
    for (int mask = 1; mask < (1 << item_cnt); ++mask) {
        value[mask] = value[mask & (mask - 1)] + items[__builtin_ctz(mask)].cost;
    }

    vector<vector<vector<State>>> labels;
    solve(n, item_cnt, start, T_lim, graph, items, items_at, labels);

    return 0;
}
