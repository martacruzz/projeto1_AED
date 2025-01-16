% simple matlab scrip to process data on a txt file and produce some graphs
% for vizualisation of growing complexity

% Base directory for saving plots
base_path = "plots"; 

if ~exist(base_path, 'dir')

    mkdir(base_path);

end

% definition of input arguments - modify here to test worst / best case
min_m = 50;
inc_m = 1;
max_m = 100;
m_array = min_m:max_m;

min_n = 25;
inc_n = 1;
max_n = 70;
n_array = min_n:max_n;

min_s = 1;
inc_s = 1;
max_s = 100;
s_array = 1:100;

%% for m variant

m_path = fullfile(base_path, 'm_variant');

if ~exist(m_path, 'dir')

    mkdir(m_path);

end

% execute and read data file
status = system(sprintf("./execute_imageChessboardTest.sh %c %d %d %d %d %d",'m', min_m, inc_m, max_m, max_n, min_s));

file = fopen("data_imageChessboardTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
time = double(data{1});
calltime = double(data{2});
pixmem = double(data{3});
numruns = double(data{4});
memspace = double(data{5});

% number of runs in function of m
f1_runs = figure(1);
plot(m_array(1:end-1), numruns, 'o-')
xlabel("m (number of rows)")
ylabel("number of runs")
title("Number of runs in function of number of rows")
legend("complexity")
grid on

% save to file
exportgraphics(f1_runs, fullfile(m_path, 'numruns_vs_m.pdf'));

% memspace in function of m
f1_mem = figure(2);
plot(m_array(1:end-1), memspace, 'o-');
xlabel("m (number of rows)")
ylabel("space in memory")
title("Space in memory in function of number of rows")
legend("complexity")
grid on

%save to file
exportgraphics(f1_mem, fullfile(m_path, 'memspace_vs_m.pdf'));

%% for n variant

n_path = fullfile(base_path, 'n_variant');

if ~exist(n_path, 'dir')

    mkdir(n_path);

end

% execute and read data file
status = system(sprintf("./execute_imageChessboardTest.sh %c %d %d %d %d %d",'n', min_n, inc_n, max_n, min_m, min_s));

file = fopen("data_imageChessboardTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
numruns = double(data{4});
memspace = double(data{5});

% number of runs in function of n
f2_runs = figure(3);
plot(n_array(1:end-1), numruns, 'o-')
xlabel("n (number of columns)")
ylabel("number of runs")
title("Number of runs in function of number of columns")
legend("complexity")
grid on

%save to file
exportgraphics(f2_runs, fullfile(n_path, 'numruns_vs_n.pdf'));

% memspace in function of n
f2_mem = figure(4);
plot(n_array(1:end-1), memspace, 'o-');
xlabel("n (number of columns)")
ylabel("space in memory")
title("Space in memory in function of number of columns")
legend("complexity")
grid on

%save to file
exportgraphics(f2_mem, fullfile(n_path, 'memspace_vs_n.pdf'));

%% for s variant

s_path = fullfile(base_path, 's_variant');

if ~exist(s_path, 'dir')

    mkdir(s_path);

end

% execute and read data file
status = system(sprintf("./execute_imageChessboardTest.sh %c %d %d %d %d %d",'s', min_s, inc_s, max_s, max_m, max_n));

file = fopen("data_imageChessboardTest.txt", "r");
formatSpec = '%15.6f\t%15.6f\t%15d\t%15d\t%15d\t%15d';
data = textscan(file, formatSpec, 'HeaderLines',1);
fclose(file);

% process variables
time = double(data{1});
calltime = double(data{2});
pixmem = double(data{3});
numruns = double(data{4});
memspace = double(data{5});

% number of runs in function of n
f3_runs = figure(5);
plot(s_array(1:end-1), numruns, 'o-')
xlabel("s (square edge)")
ylabel("number of runs")
title("Number of runs in function of square edge value")
legend("complexity")
grid on

%save to file
exportgraphics(f3_runs, fullfile(s_path, 'numruns_vs_s.pdf'));

% memspace in function of n
f3_mem = figure(6);
plot(s_array(1:end-1), memspace, 'o-');
xlabel("s (square edge)")
ylabel("space in memory")
title("Space in memory in function of square edge value")
legend("complexity")
grid on

%save to file
exportgraphics(f3_mem, fullfile(s_path, 'memspace_vs_s.pdf'));