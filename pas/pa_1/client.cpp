/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Davis Whitener
	UIN: 934001770
	Date: 9/22/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"

// include wait() command
#include <sys/wait.h>

using namespace std;


int main (int argc, char *argv[]) {

	// adding fork functionality
	if(fork() == 0) {
		char* args[] = {(char*)"./server", nullptr};
		execvp(args[0], args);
	}

	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

	// making csv
	ofstream output("x1.csv");

	// preveting errors
	datamsg error_preventer(p, t, e);

	char buf[MAX_MESSAGE]; // 256

	// getting 1000 points
	for (int i = 0; i < 10; ++i) {
		double time = i * .004;

		datamsg one(p, time, 1);
		memcpy(buf, &one, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double ecg1;
		chan.cread(&ecg1, sizeof(double)); //answer

		datamsg two(p, time, 2);
		memcpy(buf, &two, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double ecg2;
		chan.cread(&ecg2, sizeof(double)); //answer
		
		output << time << "," << ecg1 << "," << ecg2 << "\n";
	}
	output.close();

    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

	wait(NULL);
}
