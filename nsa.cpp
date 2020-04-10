#include <cstdlib>
#include <iostream>
#include <cmath>
#include <random>
#include <set>
#include <string>
#include <vector>
#include "csv.hpp"

typedef float datatype;

constexpr int problem_size = 20;
double search_space[problem_size][problem_size];

void init_search_space(void)
{
    for (int i = 0; i < problem_size; i++) {
        search_space[i][0] = 0.0;
        search_space[i][1] = 1.0;
    }
}

std::vector<datatype *> *read_dataset(const char *filename)
{
    std::vector<datatype *> *data = new std::vector<datatype *>();
    std::vector<std::string> none(problem_size, "none");
    csv::CSVFormat format;
    format.column_names(none);
    csv::CSVReader reader(filename, format);
    for (csv::CSVRow &row : reader) {
        datatype *row_data = new datatype[problem_size];
        int row_count = 0;
        for (csv::CSVField &field : row) {
            row_data[row_count] = field.get<datatype>();
            row_count++;
        }
        data->push_back(row_data);
    }
    return data;
}

std::random_device r;
std::default_random_engine engine(r());
std::uniform_real_distribution<datatype> uniform(0.0, 1.0);

void random_vector(datatype *vector)
{
    for (int i = 0; i < problem_size; i++) {
        vector[i] = search_space[i][0] + ((search_space[i][1] - search_space[i][0]) * uniform(engine));
    }
}

datatype euclidean_distance(datatype *vector, datatype *points) {
    double sum = 0;
    for (int i = 0; i < problem_size; i++) {
        double tmp = vector[i] * points[i];
        sum += (tmp * tmp);
    }
    return std::sqrt(sum);
}

bool matches(datatype *vector, std::vector<datatype *> *dataset, int min_dist)
{
    for (auto it = dataset->cbegin(); it != dataset->cend(); it++) {
        datatype dist = euclidean_distance(vector, *it);
        if(dist <= min_dist) {
            return true;
        }
    }
    return false;
}

std::vector<datatype*> *generate_detectors(int max_detectors, std::vector<datatype *> *self_dataset, int min_dist, int generation)
{
    std::vector<datatype *> *detectors = new std::vector<datatype *>();
    std::cout << "Generating detectors..." << std::endl;
    datatype *detector = new datatype[problem_size];
    int count = 1;
    do {
        random_vector(detector);
        if (!matches(detector, self_dataset, min_dist)) {
            if (!matches(detector, detectors, 0.0)) {
                detectors->push_back(detector);
                detector = new datatype[problem_size];
            }
            std::cout << count++ << "/" << max_detectors << std::endl;
        }
    } while (detectors->size() < max_detectors);
    if (detector != *detectors->cend()) {
        delete[] detector;
    }
    return detectors;
}

std::string apply_detectors(std::vector<datatype*> *detectors, std::vector<datatype *> *self_dataset, int min_dist)
{
    std::set<int> *detected = new std::set<int>();
    int trial = 1;
    for (auto it = self_dataset->cbegin(); it != self_dataset->cend(); it++) {
        bool actual = !matches(*it, detectors, min_dist);
        bool expected = matches(*it, self_dataset, min_dist);
        if (actual == expected) {
            detected->insert(trial);
        }
        trial++;
    }
    int values[] = {122, 142, 249, 269, 409, 437, 462, 468, 510, 539, 588, 618, 678, 692, 739, 742, 795};

    std::set<int> expected_detected(std::begin(values), std::end(values));
    datatype expected_detected_size = expected_detected.size();
    for (auto it = detected->cbegin(); it != detected->cend(); it++) {
        auto found = expected_detected.find(*it);
        if (found != expected_detected.end()) {
            expected_detected.erase(*it);
        }
    }
    datatype new_expected_detected_size = expected_detected.size();
    std::string results = std::to_string(-(new_expected_detected_size - expected_detected_size) / expected_detected_size);

    expected_detected.clear();
    expected_detected.insert(std::begin(values), std::end(values));
    for (auto it = expected_detected.cbegin(); it != expected_detected.cend(); it++) {
        auto found = detected->find(*it);
        if (found != detected->end()) {
            detected->erase(*it);
        }
    }
    datatype new_detected_size = detected->size();
    results += ", " + std::to_string(new_detected_size / expected_detected_size);

    return results + "\n";
}

void run(int max_detectors, int min_dist, int amount_of_proofs)
{
    std::string general_results("");

    init_search_space();

    for (int proof = 0; proof < amount_of_proofs; proof++) {
        std::vector<datatype*>* self_dataset_for_training = read_dataset("cortex_training.csv");
        std::vector<datatype*>* generate_self_dataset_for_testing = read_dataset("cortex_testing.csv");
        std::vector<datatype*> *detectors = generate_detectors(max_detectors, self_dataset_for_training, min_dist, proof + 1);
        general_results.append(apply_detectors(detectors, generate_self_dataset_for_testing, min_dist));
    }

    std::cout << general_results << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    int max_detectors, min_dist, amount_of_proofs;

    if (argc == 4) {
        max_detectors = std::atoi(argv[1]);
        min_dist = std::atoi(argv[2]);
        amount_of_proofs = std::atoi(argv[3]);
    } else if (argc == 1) {
        max_detectors = 160000;
        min_dist = 4;
        amount_of_proofs = 1;
    } else {
        std::cout << "Usage: " << argv[0] << " [MAX-DETECTORS] [MIN-DISTANCE] [AMOUNT-OF-POOFS]" << std::endl
                  << std::endl;
        return -1;
    }

    run(max_detectors, min_dist, amount_of_proofs);

    return 0;
}
