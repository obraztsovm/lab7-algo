#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

typedef struct {
    int* items;
    int size;
    double centroid;
    double variance;
} Cluster;

typedef struct {
    Cluster* clusters;
    int k;
    double score;
} ClusteringResult;

double calculateCentroid(int* items, int size, double* data)
{
    if (size == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[items[i]];
    }
    return sum / size;
}

double calculateClusterVariance(int* items, int size, double* data, double centroid)
{
    if (size == 0) return 0.0;
    double variance = 0.0;
    for (int i = 0; i < size; i++) {
        double diff = data[items[i]] - centroid;
        variance += diff * diff;
    }
    return variance;
}

double calculateTotalVariance(ClusteringResult* result, double* data)
{
    (void)data;
    double total = 0.0;
    for (int i = 0; i < result->k; i++) {
        total += result->clusters[i].variance;
    }
    return total;
}

void copyResult(ClusteringResult* dest, ClusteringResult* src, int n, int k)
{
    (void)n;
    dest->k = k;
    dest->score = src->score;
    dest->clusters = (Cluster*)malloc(k * sizeof(Cluster));

    for (int i = 0; i < k; i++) {
        dest->clusters[i].size = src->clusters[i].size;
        dest->clusters[i].centroid = src->clusters[i].centroid;
        dest->clusters[i].variance = src->clusters[i].variance;
        dest->clusters[i].items = (int*)malloc(src->clusters[i].size * sizeof(int));
        memcpy(dest->clusters[i].items, src->clusters[i].items,
               src->clusters[i].size * sizeof(int));
    }
}

void freeResult(ClusteringResult* result, int k)
{
    for (int i = 0; i < k; i++) {
        free(result->clusters[i].items);
    }
    free(result->clusters);
}

void generateAllPartitions(double* data, int n, int k, int index,
                           int* assignment, ClusteringResult* best,
                           int* call_count, double* best_score)
{
    (*call_count)++;

    if (index == n) {
        for (int i = 0; i < k; i++) {
            int count = 0;
            for (int j = 0; j < n; j++) {
                if (assignment[j] == i) count++;
            }
            if (count == 0) return;
        }

        Cluster* clusters = (Cluster*)calloc(k, sizeof(Cluster));
        for (int i = 0; i < k; i++) {
            clusters[i].items = (int*)malloc(n * sizeof(int));
            clusters[i].size = 0;
        }

        for (int i = 0; i < n; i++) {
            int cluster_id = assignment[i];
            clusters[cluster_id].items[clusters[cluster_id].size++] = i;
        }

        double total_variance = 0.0;
        for (int i = 0; i < k; i++) {
            clusters[i].centroid = calculateCentroid(clusters[i].items,
                                                      clusters[i].size, data);
            clusters[i].variance = calculateClusterVariance(clusters[i].items,
                                                             clusters[i].size,
                                                             data,
                                                             clusters[i].centroid);
            total_variance += clusters[i].variance;
        }

        if (total_variance < *best_score) {
            *best_score = total_variance;
            best->k = k;
            best->score = total_variance;

            if (best->clusters != NULL) {
                for (int i = 0; i < k; i++) {
                    free(best->clusters[i].items);
                }
                free(best->clusters);
            }

            best->clusters = (Cluster*)malloc(k * sizeof(Cluster));
            for (int i = 0; i < k; i++) {
                best->clusters[i].size = clusters[i].size;
                best->clusters[i].centroid = clusters[i].centroid;
                best->clusters[i].variance = clusters[i].variance;
                best->clusters[i].items = (int*)malloc(clusters[i].size * sizeof(int));
                memcpy(best->clusters[i].items, clusters[i].items,
                       clusters[i].size * sizeof(int));
            }
        }

        for (int i = 0; i < k; i++) {
            free(clusters[i].items);
        }
        free(clusters);

        return;
    }

    for (int i = 0; i < k; i++) {
        assignment[index] = i;
        generateAllPartitions(data, n, k, index + 1, assignment,
                              best, call_count, best_score);
    }
}

ClusteringResult* cluster(double* data, int n, int k)
{
    if (k > n) {
        printf("Error: number of clusters exceeds number of elements\n");
        return NULL;
    }

    ClusteringResult* result = (ClusteringResult*)malloc(sizeof(ClusteringResult));
    result->clusters = NULL;
    result->k = k;
    result->score = 0.0;

    int* assignment = (int*)malloc(n * sizeof(int));
    double best_score = 1e100;
    int call_count = 0;

    printf("Starting exhaustive search...\n");
    printf("Elements: %d, clusters: %d\n", n, k);
    printf("Total partitions: %d\n", (int)pow(k, n));

    clock_t start = clock();
    generateAllPartitions(data, n, k, 0, assignment, result, &call_count, &best_score);
    clock_t end = clock();

    double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    printf("Search completed!\n");
    printf("Execution time: %.3f ms\n", time_ms);
    printf("Recursive calls: %d\n", call_count);
    printf("Best total variance: %.6f\n\n", best_score);

    free(assignment);

    return result;
}

void printClusteringResult(ClusteringResult* result, double* data, int n)
{
    (void)n;
    printf("Clustering result:\n");
    for (int i = 0; i < result->k; i++) {
        printf("Cluster %d (centroid = %.3f, variance = %.3f): [",
               i + 1, result->clusters[i].centroid, result->clusters[i].variance);
        for (int j = 0; j < result->clusters[i].size; j++) {
            int idx = result->clusters[i].items[j];
            printf("%.0f", data[idx]);
            if (j < result->clusters[i].size - 1) printf(", ");
        }
        printf("]\n");
    }
    printf("Total variance: %.6f\n", result->score);
}

double measureTime(double* data, int n, int k)
{
    clock_t start = clock();
    ClusteringResult* result = cluster(data, n, k);
    clock_t end = clock();

    double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    if (result) {
        freeResult(result, k);
        free(result);
    }

    return time_ms;
}

int main(void)
{
    printf("\n Clustering with exhaustive search\n\n");

    printf("Experiment 1: n=5, k=2\n");
    double data1[] = {1, 2, 3, 10, 11};
    ClusteringResult* result1 = cluster(data1, 5, 2);
    if (result1) {
        printClusteringResult(result1, data1, 5);
        freeResult(result1, 2);
        free(result1);
    }
    printf("\n");

    printf("Experiment 2: n=7, k=2\n");
    double data2[] = {1, 2, 3, 4, 10, 11, 12};
    ClusteringResult* result2 = cluster(data2, 7, 2);
    if (result2) {
        printClusteringResult(result2, data2, 7);
        freeResult(result2, 2);
        free(result2);
    }
    printf("\n");

    printf("Experiment 3: n=10, k=2\n");
    double data3[] = {1, 2, 3, 4, 5, 10, 11, 12, 13, 14};
    ClusteringResult* result3 = cluster(data3, 10, 2);
    if (result3) {
        printClusteringResult(result3, data3, 10);
        freeResult(result3, 2);
        free(result3);
    }
    printf("\n");

    printf("Experiment 4: n=5, k=3\n");
    double data4[] = {1, 2, 8, 9, 15};
    ClusteringResult* result4 = cluster(data4, 5, 3);
    if (result4) {
        printClusteringResult(result4, data4, 5);
        freeResult(result4, 3);
        free(result4);
    }
    printf("\n");

    printf("Experiment 5: n=7, k=3\n");
    double data5[] = {1, 2, 3, 8, 9, 10, 15};
    ClusteringResult* result5 = cluster(data5, 7, 3);
    if (result5) {
        printClusteringResult(result5, data5, 7);
        freeResult(result5, 3);
        free(result5);
    }
    printf("\n");

    printf("Experiment 6: n=5, k=4\n");
    double data6[] = {1, 3, 5, 7, 9};
    ClusteringResult* result6 = cluster(data6, 5, 4);
    if (result6) {
        printClusteringResult(result6, data6, 5);
        freeResult(result6, 4);
        free(result6);
    }
    printf("\n");

    printf("Time measurement table:\n");
    printf("%-10s %-10s %-15s\n", "N", "K", "Time (ms)");

    double test_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int test_sizes[] = {5, 5, 5, 7, 7, 10};
    int test_ks[] = {2, 3, 4, 2, 3, 2};

    for (int i = 0; i < 6; i++) {
        double t = measureTime(test_data, test_sizes[i], test_ks[i]);
        printf("%-10d %-10d %-15.3f\n", test_sizes[i], test_ks[i], t);
    }

    printf("\nExtrapolation for larger N:\n");
    printf("Estimated time for N=15, K=2: %.0f ms (%.1f sec)\n",
           100.0 * pow(2, 15) / pow(2, 10), 100.0 * pow(2, 15) / pow(2, 10) / 1000);
    printf("Estimated time for N=20, K=2: %.0f ms (%.1f sec)\n",
           100.0 * pow(2, 20) / pow(2, 10), 100.0 * pow(2, 20) / pow(2, 10) / 1000);

    return 0;
}