#include <leveldb/db.h>
#include <string>
#include <iostream>
#include <cstring>

#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads/";

const string load = workload + "10w-rw-25-75-load.txt"; // TODO: the workload_load filename
const string run  = workload + "10w-rw-25-75-run.txt"; // TODO: the workload_run filename

const string filePath = "leveldb";

const int READ_WRITE_NUM = 100000; // TODO: how many operations

int main()
{        
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;
    // TODO: open and initial the levelDB

    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, filePath , &db);
    if(!s.ok()){
        printf("Open LevelDB Error!\n");
        return 0;
    }


    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000]; // the key and value are same
    bool* ifInsert = new bool[2200000]; // the operation is insertion or not
	FILE *ycsb_load, *ycsb_run; // the files that store the ycsb operations
	char *buf = NULL;
	size_t len = 0;
    struct timespec start, finish; // use to caculate the time
    double single_time; // single operation time

    printf("Load phase begins \n");
    // TODO: read the ycsb_load and store
    // load only insert

    ycsb_load = fopen((char*)load.c_str(), "r");
    if (ycsb_load == NULL)
    {
        cout << "ycsb_load open fail!" << endl;
        return 0;
    }
    char op[7];
    for (int i = 0; i < READ_WRITE_NUM; ++i)
    {
        fscanf(ycsb_load, "%s %lu", op, &key[i]);
        if (strcmp(op, "INSERT") == 0)
        {
            ifInsert[i] = true;
        }
    }


    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in LevelDB

    for (int i = 0; i < READ_WRITE_NUM; ++i) {
        char dbkey;
        string value;
        memcpy(&dbkey,&key[i],8);
        if (ifInsert[i]) {
            ++inserted;
            db->Put(write_options, &dbkey, &dbkey);
        } else {
            ++queried;
            db->Get(leveldb::ReadOptions(), &dbkey, &value);
        }
    }


    clock_gettime(CLOCK_MONOTONIC, &finish);


	single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);

    printf("Load phase finishes: %lu items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

	int operation_num = 0;
    inserted = 0;		

    // TODO:read the ycsb_run and store

    ycsb_load = fopen((char*)load.c_str(), "r");
    if (ycsb_load == NULL)
    {
        cout << "ycsb_load open fail!" << endl;
        return 0;
    }
    char op[7];
    for (int i = 0; i < READ_WRITE_NUM; ++i)
    {
        fscanf(ycsb_load, "%s %lu", op, &key[i]);
        if (strcmp(op, "INSERT") == 0)
        {
            ifInsert[i] = true;
        }
    }



    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the levelDB

    for (int i = 0; i < READ_WRITE_NUM; ++i) {
        char dbkey;
        string value;
        memcpy(&dbkey,&key[i],8);
        if (ifInsert[i]) {
            ++inserted;
            db->Put(write_options, &dbkey, &dbkey);
        } else {
            ++queried;
            db->Get(read_options, &dbkey, &value);
        }
    }



	clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %lu/%lu items are inserted/searched\n", operation_num - inserted, inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    return 0;
}
