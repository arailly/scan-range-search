#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <arailib.hpp>

using namespace std;
using namespace arailib;

struct ScanResult {
    Series series;
    size_t n_visit_points = 0;
    time_t time = 0;
    ScanResult() : n_visit_points(0), time(0) {}
};

ScanResult scan_range_search(const Point query, const float range, const Series& series,
                             const string& distance = "euclidean") {
    const auto start = chrono::system_clock::now();

    auto result = ScanResult();

    const auto distance_function = [&]() {
        if (distance == "euclidean") return euclidean_distance;
        if (distance == "angular") return angular_distance;
        else throw runtime_error("invalid distance.");
    }();

    for (const auto &point : series) {
        if (distance_function(query, point) < range) result.series.push_back(point);
    }

    result.n_visit_points = series.size();

    const auto end = chrono::system_clock::now();
    result.time = chrono::duration_cast<chrono::microseconds>(end - start).count();

    return result;
}

void write_results(const string& save_path, const vector<ScanResult>& results) {
    ofstream ofs(save_path);
    string line = "time,n_result,n_visit_points\n";
    ofs << line;

    for (const auto& result : results) {
        line = to_string(result.time) + "," +
               to_string(result.series.size()) + "," +
               to_string(result.n_visit_points) + "\n";
        ofs << line;
    }
}

int main() {
    const auto config = read_config();
    const int n = config["n"];
    const int n_queries = config["n_queries"];
    const float range = config["range"];
    const string distance = config["distance"];
    const string data_path = config["data_path"];
    const string query_path = config["query_path"];
    const string save_path = config["save_path"];

    const auto series = load_data(data_path, n);
    const auto queries = read_csv(query_path, n_queries);

    cout << "complete: load data and index" << endl;

    vector<ScanResult> results(queries.size());
    for (int i = 0; i < queries.size(); i++) {
        const auto& q = queries[i];
        const auto r = scan_range_search(q, range, series, distance);
        results[i] = r;
    }

    cout << "complete: search" << endl;

    write_results(save_path, results);
}