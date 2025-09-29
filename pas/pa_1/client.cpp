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
	bool new_channel = false;
	
	string filename = "";

	while ((opt = getopt(argc, argv, "p:t:e:f:c")) != -1) {
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
			case 'c':
				new_channel = true;
				break;
		}
	}

    FIFORequestChannel control_chan("control", FIFORequestChannel::CLIENT_SIDE);
	FIFORequestChannel* chan;

	// requesting new channel
	if (new_channel) {
		MESSAGE_TYPE msg = NEWCHANNEL_MSG;
		control_chan.cwrite(&msg, sizeof(MESSAGE_TYPE));
		char new_channel_name[50];
		int bytes_read = control_chan.cread(new_channel_name, sizeof(new_channel_name));
		new_channel_name[bytes_read] = '\0';

		chan = new FIFORequestChannel(new_channel_name, FIFORequestChannel::CLIENT_SIDE);
	} else {
		chan = &control_chan;
	}
	
	// making csv
	ofstream output("received/x1.csv");

	// preveting errors
	// datamsg error_preventer(p, t, e);

	char buf[MAX_MESSAGE]; // 256

	// getting single data point
    datamsg x(p, t, e);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan->cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan->cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;

	// getting 1000 points
	for (int i = 0; i < 0; ++i) {
		double time = i * .004;

		datamsg one(p, time, 1);
		memcpy(buf, &one, sizeof(datamsg));
		chan->cwrite(buf, sizeof(datamsg)); // question
		double ecg1;
		chan->cread(&ecg1, sizeof(double)); //answer

		datamsg two(p, time, 2);
		memcpy(buf, &two, sizeof(datamsg));
		chan->cwrite(buf, sizeof(datamsg)); // question
		double ecg2;
		chan->cread(&ecg2, sizeof(double)); //answer
		
		output << time << "," << ecg1 << "," << ecg2 << "\n";
	}
	output.close();

    if (!filename.empty()) {
		filemsg fm(0, 0);

		__int64_t file_length;

		int len = sizeof(filemsg) + (filename.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), filename.c_str());
		chan->cwrite(buf2, len);  // I want the file length;
		chan->cread(&file_length, sizeof(__int64_t)); // store length in file_length
		cout << file_length;

		delete[] buf2;

		ofstream file_output("received/" + filename);
		int file_chunk = MAX_MESSAGE;
		int file_offset = 0;
		// holds file chunk
		char* data_chunk = new char[MAX_MESSAGE];

		while(file_offset < file_length) {
			int msg_size;
			// condition for if message is smaller than 256 bytes
			if (file_offset + file_chunk > file_length) {
				msg_size = file_length - file_offset;
			}
			else {
				msg_size = file_chunk;
			}

			filemsg fm(file_offset, msg_size);

			int len = sizeof(filemsg) + (filename.size() + 1);
			char* buf2 = new char[len];
			memcpy(buf2, &fm, sizeof(filemsg));
			strcpy(buf2 + sizeof(filemsg), filename.c_str());
			chan->cwrite(buf2, len); 

			chan->cread(data_chunk, msg_size);
			file_output.write(data_chunk, msg_size);
			file_offset += file_chunk;

			delete[] buf2;
		}

		file_output.close();
		delete[] data_chunk;
	}
	
	// closing the channel / channels if new channel is used  
    MESSAGE_TYPE m = QUIT_MSG;

	if (new_channel) {
		chan->cwrite(&m, sizeof(MESSAGE_TYPE));
	}
	control_chan.cwrite(&m, sizeof(MESSAGE_TYPE));
	
	if (new_channel) {
		delete chan;
	}

	wait(nullptr);
}
