% simple matlab scrip to process data on a txt file and produce some graphs
% for vizualisation of growing complexity

% Base directory for saving plots
base_path = "plots"; 

if ~exist(base_path, 'dir')

    mkdir(base_path);

end

% definition of input arguments
min_m = 1;
inc_m = 1;
max_m = 50;
m_array = 1:50;

min_n = 1;
inc_n = 1;
max_n = 70;
n_array = 1:70;

s = 1;

version1 = "V1";
version2 = "V2";

%% for m variant

m_path = fullfile(base_path, 'm_variant');

if ~exist(m_path, 'dir')

    mkdir(m_path);

end

%%% for version 1 (optimized)

% execute and read data file
status = system(sprintf("./execute_imageANDTest.sh %c %d %d %d %d %s",'m', min_m, inc_m, max_m, max_n, version1));

file = fopen("data_imageANDTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
nops = double(data{6});

% number of runs in function of m
f1_runs = figure(1);
plot(m_array(1:end-1), nops, 'o-')
xlabel("m (number of rows)")
ylabel("number of operations")
title("Number of operations in function of number of rows")
grid on

hold on


%%% for version 2 (non-optimized)

% execute and read data file
status = system(sprintf("./execute_imageANDTest.sh %c %d %d %d %d %s",'m', min_m, inc_m, max_m, max_n, version2));

file = fopen("data_imageANDTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
nops = double(data{6});

% number of runs in function of m
plot(m_array(1:end-1), nops, 'o-')
xlabel("m (number of rows)")
ylabel("number of operations")
title("Number of operations in function of number of rows")
legend("Optimized Version","Non-optimized Version")
grid on

hold off

% save to file
exportgraphics(f1_runs, fullfile(m_path, 'nops_vs_m.pdf'));

%% for n variant

n_path = fullfile(base_path, 'n_variant');

if ~exist(n_path, 'dir')

    mkdir(n_path);

end

%%% for version1

% execute and read data file
status = system(sprintf("./execute_imageANDTest.sh %c %d %d %d %d %s",'n', min_n, inc_n, max_n, max_m, version1));

file = fopen("data_imageANDTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
nops = double(data{6});

% number of runs in function of n
f2_runs = figure(2);
plot(n_array(1:end-1), nops, 'o-')
xlabel("n (number of columns)")
ylabel("number of operations")
title("Number of operations in function of number of columns")
grid on

hold on

%%% for version2

% execute and read data file
status = system(sprintf("./execute_imageANDTest.sh %c %d %d %d %d %s",'n', min_n, inc_n, max_n, max_m, version2));

file = fopen("data_imageANDTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
nops = double(data{6});

% number of runs in function of n
plot(n_array(1:end-1), nops, 'o-')
xlabel("n (number of columns)")
ylabel("number of operations")
title("Number of operations in function of number of columns")
legend("Optimized Version","Non-optimized Version")
grid on

hold off

%save to file
exportgraphics(f2_runs, fullfile(n_path, 'numruns_vs_n.pdf'));
