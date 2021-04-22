#include <iostream> //This program was developed in Visual Studio 2017.
#include <ctime>
#include <conio.h>
using namespace std;
int count_rows(char *directory) { //Counts lines in a given text file to determine array's size for dynamic allocation.
	FILE *fp;
	fopen_s(&fp, directory, "r");
	int line_count = 0;
	char chr;
	if (fp == NULL) perror("ERROR");
	else {
		chr = getc(fp);
		while (chr != EOF) {
			if (chr == '\n') line_count++;
			chr = getc(fp);
		}
		fclose(fp);
	}
	return line_count;
}
int find_column_count(char *directory, int line_count) { //Finds the column count.
	int count = 0;
	FILE *fp;
	fopen_s(&fp, directory, "r");
	if (fp == NULL) perror("ERROR");
	else {
		char line[300];
		char *token = NULL;
		char *context = NULL;
		fgets(line, sizeof line, fp);
		token = strtok_s(line, ",", &context); //Seperates first line by commas.
		do {
			count++;
			token = strtok_s(NULL, ",", &context);
		} while (token != NULL);
		fclose(fp);
	}
	return count;
}
int find_max_string_length(char *directory, int line_count) { //Finds longest string length to determine array's size for minimizing dynamic allocated memory.
	int *length_list_array = new int[line_count];
	FILE *fp;
	fopen_s(&fp, directory, "r");
	if (fp == NULL) perror("ERROR");
	else {
		int count = 0;
		char line[300];
		char *token = NULL;
		char *context = NULL;
		while (fgets(line, sizeof line, fp) != NULL) {
			token = strtok_s(line, "\n", &context); //Discards new line character from each line of the text file.
			length_list_array[count++] = strlen(token);
		}
		fclose(fp);
	}
	int max = 0;
	for (int i = 0; i < line_count; i++) {
		if (length_list_array[i] > max) max = length_list_array[i];
	}
	delete length_list_array;
	return max;
}
void read_data_csv(char *directory, int max_length, double **data_array) { //Reads csv file and inserts into a 2D integer array.
	FILE *fp;
	fopen_s(&fp, directory, "r");
	if (fp == NULL) perror("ERROR");
	else {
		char *line = new char[max_length + 2]; //Implements dynamic allocation in a sufficient size which is the result of "string_length + 1('\n') + 1('\0')".
		char *token = NULL;
		char *context = NULL;
		int column;
		int row = 0;
		while (fgets(line, (max_length + 2), fp) != NULL) {
			column = 0;
			token = strtok_s(line, ",", &context); //Seperates line by commas.
			do {
				data_array[row][column++] = atof(token);
				token = strtok_s(NULL, ",", &context);
			} while (token != NULL);
			row++;
		}
		delete line;
		fclose(fp);
	}
}
void read_truth_csv(char *directory, int max_length, int **correct_result) { //Reads csv file and inserts into a 2D integer array.
	FILE *fp;
	fopen_s(&fp, directory, "r");
	if (fp == NULL) perror("ERROR");
	else {
		char *line = new char[max_length + 2]; //Implements dynamic allocation in a sufficient size which is the result of "string_length + 1('\n') + 1('\0')".
		char *token = NULL;
		char *context = NULL;
		int column = 0;
		while (fgets(line, (max_length + 2), fp) != NULL) {
			token = strtok_s(line, "\n", &context); //Seperates line by commas.
			correct_result[column++][0] = atoi(token);
		}
		delete line;
		fclose(fp);
	}
}
double euclidian_distance(double **data_array, double **centroid, int centroid_index, int cell_index, int feature_count) { //Calculates Euclidian distance.
	//Formula: sqrt((x(centroid) - x(index_cell))^2 + (y(centroid) - y(index_cell))^2 + (z(centroid) - z(index_cell))^2 + ...)
	double sum = 0;
	for (int i = 0; i < feature_count; i++) {
		sum = sum + pow((centroid[centroid_index][i] - data_array[cell_index][i]), 2);
	}
	return sqrt(sum);
}
int nearest_centroid(double **euclidian_distance_array, int euclidian_distance_array_index, int k) { //Finds nearest centroid's index.
	int index = euclidian_distance_array_index; //For writing shortly.
	int nearest = 0; //Nearest centroid's number (For initialization is 0.).
	for (int i = 1; i < k; i++) if (euclidian_distance_array[index][i] < euclidian_distance_array[index][nearest]) nearest = i;
	return nearest;
}
void update_centroids(double **data_array, double **centroid, int **result, int row_count, int column_count, int k) { //Updates centroids' data.
	int *member_count = new int[k];
	for (int i = 0; i < k; i++) member_count[i] = 0; //Initiates member_count array.
	for (int i = 0; i < row_count; i++) {
		for (int j = 0; j < k; j++) {
			if (result[i][0] == j) {
				member_count[j]++; //Counts members of clusters.
				break;
			}
		}
	}
	int **member_indexes = new int*[k];
	for (int i = 0; i < k; i++) member_indexes[i] = new int[member_count[i]]; //Dynamic allocation of members' indexes' array.
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < member_count[i]; j++) {
			for (int m = 0; m < row_count; m++) {
				if (result[m][0] == i) member_indexes[i][j] = m; //Assigns clusters' members' indexes into member_indexes array.
			}
		}
	}
	double sum;
	for (int i = 0; i < k; i++) { //For every centroid implements the formula which is "(x(member_1) + x(member_2) + x(member_3) + ...) /member_count".
		for (int j = 0; j < column_count; j++) {
			sum = 0; //Resets sum.
			for (int m = 0; m < member_count[i]; m++) {
				sum = sum + data_array[member_indexes[i][m]][j]; //Sums members's every feauture values
			}
			centroid[i][j] = sum / member_count[i]; //Calcucates and assigns means into centroid array.
		}
	}
	delete member_count;
	for (int i = 0; i < k; i++) delete member_indexes[i];
	delete member_indexes;
}
void k_means(double **data_array, int row_count, int column_count, int k, int **result) {
	srand((unsigned int)time(NULL));
	int *index_proto_centroid = new int[k];
	for (int i = 0; i < k; i++) {
		index_proto_centroid[i] = rand() % row_count; //Selects random cells from given data.
	}
	double **centroid = new double*[k];
	for (int i = 0; i < k; i++) centroid[i] = new double[column_count]; //Dynamic allocation of centroid array to save centroid data.
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < column_count; j++) {
			centroid[i][j] = data_array[index_proto_centroid[i]][j]; //Assigns first centroid data into centroid array.
		}
	}
	double **prev_centroid = new double*[k];
	for (int i = 0; i < k; i++) prev_centroid[i] = new double[column_count]; //Dynamic allocation of previous centroid array to control equality of previous centroids and current centroids.
	double **euclidian_distance_array = new double*[row_count];
	for (int i = 0; i < row_count; i++) euclidian_distance_array[i] = new double[k]; //Dynamic allocation of Euclidian distance array to save Euclidian distances.
	bool equality;
	do {
		for (int i = 0; i < row_count; i++) {
			for (int j = 0; j < k; j++) {
				euclidian_distance_array[i][j] = euclidian_distance(data_array, centroid, j, i, column_count);
			}
		}
		for (int i = 0; i < row_count; i++) {
			result[i][0] = nearest_centroid(euclidian_distance_array, i, k); //Updates results
		}
		for (int i = 0; i < k; i++) {
			for (int j = 0; j < column_count; j++) {
				prev_centroid[i][j] = centroid[i][j]; //Copies centroids' data before updating.
			}
		}
		update_centroids(data_array, centroid, result, row_count, column_count, k); //Updates centroids.
		equality = true;
		for (int i = 0; i < k; i++) { //Controlls equality of prev centroid and updated centroid.
			for (int j = 0; j < column_count; j++) {
				if (prev_centroid[i][j] != centroid[i][j]) {
					equality = false;
					break;
				}
			}
			if (!equality) break;
		}
	} while (!equality); //If previous centroid and updated centroid are same, processing is breaked.
	delete index_proto_centroid;
	for (int i = 0; i < row_count; i++) delete euclidian_distance_array[i];
	delete euclidian_distance_array;
	for (int i = 0; i < k; i++) delete centroid[i];
	delete centroid;
	for (int i = 0; i < k; i++) delete prev_centroid[i];
	delete prev_centroid;
}
void heap_s_alg(int *proto_pattern, int size, int k, int *count, int **pattern){ //Creates a permutation patterns by Heap's Algorithm (Reference is https://www.geeksforgeeks.org/heaps-algorithm-for-generating-permutations/, last visited at 31.03.2021.).
	if (size == 1) {
		--(*count);
		for (int i = 0; i < k; i++) {
			pattern[(*count)][i] = proto_pattern[i]; //Add permuations into pattern array.
		}
	}
	for (int i = 0; i < size; i++) {
		heap_s_alg(proto_pattern, size - 1, k, count, pattern); //Implementing recursively to swawp original array's element properly.
		if (size % 2 == 1)
			swap(proto_pattern[0], proto_pattern[size - 1]);
		else
			swap(proto_pattern[i], proto_pattern[size - 1]);
	}
}
int calculate_factorial(int k) { //Calculates factorial.
	int factorial = 1;
	for (int i = k; i > 1; i--)factorial *= i;
	return factorial;
}
void realign(int **realigned_result_list, int **predicted_result, int **pattern, int row_count, int factorial, int k) { //Creates permutations of result table.
	for (int i = 0; i < factorial; i++) {
		for (int j = 0; j < row_count; j++) {
			for (int m = 0; m < k; m++) {
				if (predicted_result[j][0] == m) realigned_result_list[j][i] = pattern[i][m]; //Adds permutations into an array.
			}
		}
	}
}
bool detect_presence(int **predicted_result_list, int **predicted_result, int index, int row_count, int result_count){ //Detects presence of new result in result list.
	bool equality = false; //Formula is "((A1 AND A2 AND A3 AND ...) OR (B1 AND B2 AND B3 AND ...) OR (C1 AND C2 AND C3 AND ...) OR ...)"
	bool inner_equality;
	for (int i = 0; i < result_count; i++) {
		inner_equality = true;
		for (int j = 0; j < row_count; j++) {
			if (predicted_result_list[j][i] != predicted_result[j][index]) { //Detects mismatches.
				inner_equality = false;
				break;
			}
		}
		equality = equality || inner_equality;
		if (equality || inner_equality) break;
	}
	return equality; //If there is not an existence of new result in result list, it returns false. 
}
double accuracy(int **predicted_result_list, int result_number, int **correct_result, int row_count) { //Calculates accuracy.
	double correctness = 0;
	for (int i = 0; i < row_count; i++) if (predicted_result_list[i][result_number] == correct_result[i][0]) correctness++;
	double accuracy_percent = (correctness * 100) / row_count; //Calculates percent value.
	return accuracy_percent;
}
double find_max_accuracy(int **predicted_result_list, int result_count, int **correct_result, int row_count) { //Finds maximum accuracy from given result list.
	double current_accuracy;
	double max_accuracy = 0;
	for (int i = 0; i < result_count; i++) {
		current_accuracy = accuracy(predicted_result_list, i, correct_result, row_count);
		if (current_accuracy > max_accuracy) max_accuracy = current_accuracy;
	}
	return max_accuracy;
}
void print_results(int **predicted_result_list, int row_count, int result_count) { //Shows K-means results for testing.
	for (int i = 0; i < row_count; i++) {
		for (int j = 0; j < result_count; j++) {
			cout << predicted_result_list[i][j]<<" ";
		}
		cout << endl;
	}
}
int main() {
	char data_directory[] = "breast_data.csv";
	int row_count = count_rows(data_directory); //Finds row count (member count) from csv file to.
	int column_count = find_column_count(data_directory, row_count); //Finds column count (feature count) from csv file.
	double **data_array = new double*[row_count];
	for (int i = 0; i < row_count; i++) data_array[i] = new double[column_count]; //Dynamic allocation of data array (double data[569][30] for given csv file).
	int max_length_data = find_max_string_length(data_directory, row_count);
	read_data_csv(data_directory, max_length_data, data_array); //Reads data of csv file and inserts data into an array.
	int k = 2; //For breast cancer, k is 2 (Values of 5 or more are dangerous.).
	int **correct_result = new int*[row_count];
	for (int i = 0; i < row_count; i++) correct_result[i] = new int[1]; //Dynamic allocation of correct result array (int correct_result[569][1] for k = 2).
	char result_directory[] = "breast_truth.csv";
	int max_length_result = find_max_string_length(result_directory, row_count); //Finds maximum length for truth table (It's obviously 1 but for different csv files, it may change.).
	read_truth_csv(result_directory, max_length_result, correct_result); //Reads truth table and inserts data into an array.
	int **predicted_result = new int*[row_count];
	for (int i = 0; i < row_count; i++) predicted_result[i] = new int[1]; //Dynamic allocation of predicted result array (int predicted_result[569][1] for k = 2).
	int result_count = 1; //Counts different results.
	int factorial = calculate_factorial(k);
	int iteration_limit = factorial * (k + 2);
	int count = 0; //Counts counter to reach limitation.
	int *proto_origin = new int[k];
	for (int i = 0; i < k; i++) proto_origin[i] = i; //Dynamic allocation and initalization of original pattern array (int proto_origin[2] = {0, 1} for k = 2).
	int **pattern = new int*[factorial];
	for (int i = 0; i < factorial; i++) pattern[i] = new int[k]; //Dynamic allocation of pattern array (int pattern[factorial][k]).
	heap_s_alg(proto_origin, k, k, &factorial, pattern); //Implements Heap's Algorithm to crate permutations.
	factorial = calculate_factorial(k); //Recalculates factorial because in previous line, it was changed by pass by pointer operation.
	int **realigned_result_list = new int*[row_count];
	for (int i = 0; i < row_count; i++) realigned_result_list[i] = new int[factorial]; //Dynamic allocation of realligned result list (int realigned_result_list[569][2] for k =2).
	int **predicted_result_list = (int **)malloc(sizeof(int *) * row_count);
	for (int i = 0; i < row_count; i++) predicted_result_list[i] = (int *)malloc(sizeof(int) * (iteration_limit)); //Dynamic allocation of predicted result list to be prepared maximum result variations (int predicted_result_list[569][16] for k =2).
	do {
		k_means(data_array, row_count, column_count, k, predicted_result); //Implement K-means and creates result until limitation is reached.
		if(!count) for (int i = 0; i < row_count; i++) predicted_result_list[i][result_count - 1] = predicted_result[i][0];
		realign(realigned_result_list, predicted_result, pattern, row_count, factorial, k); //Creates permutations of k-means algorithm's result.
		for (int i = 0; i < factorial; i++) {
			if (!detect_presence(predicted_result_list, realigned_result_list, i, row_count, result_count)) { //If result/results are different, adds them into list.
				result_count++;
				for (int j = 0; j < row_count; j++) predicted_result_list[j][result_count - 1] = realigned_result_list[j][i];
			}
		}
		//if (!detect_presence(predicted_result_list, predicted_result, 0, row_count, result_count)) { //If result/results are different, adds them into list.
		//	result_count++;
		//	for (int i = 0; i < row_count; i++) predicted_result_list[i][result_count - 1] = predicted_result[i][0];
		//}
		count++;
	} while (count != iteration_limit);
	for (int i = 0; i < row_count; i++) predicted_result_list[i] = (int *)realloc(predicted_result_list[i], sizeof(int) * (result_count)); //Reallocation for predicted result list to minimize memory.
	//print_results(predicted_result_list, row_count, result_count); //Shows predicted results for testing.
	cout << "Accuracy: %" << find_max_accuracy(predicted_result_list, result_count, correct_result, row_count) << endl;
	for (int i = 0; i < row_count; i++) delete data_array[i];
	delete data_array;
	for (int i = 0; i < row_count; i++) delete predicted_result[i];
	delete predicted_result;
	for (int i = 0; i < row_count; i++) delete correct_result[i];
	delete correct_result;
	for (int i = 0; i < row_count; i++) free (predicted_result_list[i]);
	free (predicted_result_list);
	for (int i = 0; i < factorial; i++) delete pattern[i];
	delete pattern;
	for (int i = 0; i < row_count; i++) delete realigned_result_list[i];
	delete realigned_result_list;
	printf("Press any button to exit.");
	_getch();
	return 0;
}