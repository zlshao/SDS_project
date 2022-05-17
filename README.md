# SDS_project

1. Description:

   SDS_project is a project proposed for detecting slow port scans. Its main function is: after sampling the traffic (optional), record and extract the traffic features using the data structure SDS (Scan Detection Sketch) proposed in the paper. It can be used in two ways: (1) analyze local pcap files, and (2) capture and analyze traffic in real-time.

   SDS (Scan Detection Sketch) is a data structure customized for detecting slow port scans and can be viewed as a two-dimensional array of buckets with *d* rows and *w* columns. Each bucket of SDS consists of a combination of counters and hash tables. Each counter/hash table records a statistical feature of the traffic. When the statistics recorded by the bucket reach a specific threshold, the values in the bucket are extracted as a feature vector. Therefore, SDS can record and extract multiple features of network traffic simultaneously.

   

2. Usage:

**Usage 1** **- Analyze local pcap file**

**step1:** Modify **SDS pcap** in data.cfg;

```
SDS_in_pcap_file = "G:/2020data/public/20200603/test/202006031400.pcap.cut_type0_0-20_payload0.pcap";
SDS_random_seed = 2022;
```

The meanings of the parameters are as follows:

- SDS_in_pcap_file: the path of the pcap file to be analyzed.
- SDS_random_seed: random seed. Used to randomly select the starting point of packet sampling.

Operating environment:

Windows or Linux system

Compiling environment:

Windows: vscode + mingw 10.2.0 + cmake
Ubuntu: vscode + gcc 9.3.0 + cmake\+ libpcap

**step2**: Modify **SDS sketch sets** in data.cfg;

```
//ratio
SDS_ratio = 1; //sampling  ratio 0(1/1),1(1/8),2(1/16),3(1/32),4(1/64),5(1/128),6(1/256),7(1/512),8(1/1024),9(1/2048),10(1/4096),11(1/8192),12(1/16384),13(),14(1/65536)
//flowkey: IP+Protocol---2
SDS_SK_type = 2;
//feature
/*
statistics feature
a --- forward Pck. (1B)
b --- backward Pck.(1B)
h --- backward IP hash16 Distr. (2B)
j --- backward port hash16 Distr. (2B)
2 --- (TCP) forward SYN (1B)
3 --- (TCP) backward SYN (1B)
*/
SDS_SK_feature = "abhj23"; 
//hash bit
SDS_SK_hash_bit = 25;
//threshold
SDS_SK_threshold = 80;
```

The meanings of the parameters are as follows:

- SDS_ratio: Sampling ratio. The packets are sampled before the features are extracted. 1/8, 1/16, 1/32, and 1/64 are the recommended settings.
- SDS_SK_type: *flowkey*. *flowkey* is the key extracted from each arriving packet. The *flowkey* is used as input to the hash function, and the value output by the hash function is used to locate the buckets that should be updated in SDS. 
- SDS_SK_feature: The SDS bucket composition. The bucket composition selected for detecting slow port scans is "abhj23".
- SDS_SK_hash_bit: A parameter that is used to pre-allocate memory for SDS. The number of buckets *w* in each row of SDS is set to *2^SDS_SK_hash_bit*. The number of rows *d* of SDS is 4 by default.
- SDS_SK_threshold: The threshold for extracting feature vectors. A bucket is considered to be saturated when statistics of *SDS_SK_threshold* packets are recorded in it. If all the *d* buckets corresponding to a *flowkey* are saturated, the minimum values of the counters and hash tables in the *d* buckets are extracted as a feature vector. After extracting the features, the SDS bucket is recycled by subtracting the minimum value.  *SDS_SK_threshold* is set to 80 in the paper.

Operating environment:

Windows or Linux system

Compiling environment:

Windows: vscode + mingw 10.2.0 + cmake
Ubuntu: vscode + gcc 9.3.0 + cmake\+ libpcap

**step3:** Run the program. The compiled program and the files needed to run the program are under the **bin** path.



**Usage 2 - Capture and analyze traffic in real-time**

**step1**: Modify **SDS capture** in data.cfg;

```
SDS_dev = "enp0s31f6";
SDS_out_pcap_file = "/home/cayman/data/pcap/20210501_3.pcap";
SDS_dump_type = 2; //0 -- not dump, 1 -- only sample, 2 -- all
SDS_max_packet = 5000000;
SDS_capture_time = 900;
```

The parameter meanings are as follows.

SDS_dev: The NIC that captures the traffic.

SDS_out_pcap_file: The path and file name to save the captured traffic locally.

SDS_dump_type: Whether to dump the file locally. 0: no dump; 1: dump only the sampled traffic; 2: dump all traffic.

SDS_max_packet: Set the maximum number of packets to be captured.

SDS_capture_time: Capture time(s).

Operating environment:

Uses libpcap underlay and therefore runs on Linux systems.

Compilation environment:

Ubuntu: vscode + gcc 9.3.0 + cmake\+ libpcap

**step2**: Same as **step2** in **Usage 1**.

**step3:** Run the program. The compiled program and the files needed to run the program are under the **bin** path.



3. Machine Learning - Applying SDS_project to Port Scan Detection

The above program will output feature vectors to a **.csv file or Redis database, with each feature vector identified by *flowkey*.

To **train** a classification model for port scan detection, the following steps need to be performed:

- Acquisition of training set with ground truth.

  Background traffic acquisition: Traffic captured from the LAN. It should be ensured that all captured traffic is non-scanning traffic as much as possible.

  Port scan traffic acquisition. To train the multi-classifier, horizontal, vertical, and mixed scan traffic for TCP and UDP needs to be acquired. The paper combines two ways of acquisition:

  - Using Nmap to generate.
  - Extracting from the public dataset MAWI Working Group Traffic Archive ([Traffic Trace Info (wide.ad.jp)](http://mawi.wide.ad.jp/mawi/samplepoint-F/2021/202104101400. html)). This dataset provides a document with information on some types of port scan traffic. So the port scan traffic can be extracted from the dataset based on the IP addresses.

- Mixing the obtained background traffic with the scanned traffic and extracting the features of the mixed traffic by **Usage 1** to obtain a **.csv file.

- Labeling the feature vectors according to the *flowkey* to obtain the training set.

- Train the multi-classifier using machine learning algorithms (e.g., decision tree, random forest).

In the case of **detecting** port scanning:

- Extract traffic features by **Usage 1** or **Usage 2** according to actual requirements, which can be output to a **.csv file or Redis database.

- Inputting the features into the multi-classifier. The classifier outputs the identification results (normal/TCP horizontal scan/TCP vertical scan/TCP mixed scan/UDP horizontal scan/UDP vertical scan/UDP mixed scan).

  

  4. Test method for real-time monitoring of high-speed network traffic

  The real network environment is simulated by real-world traffic replay in Mininet, and the system topology is shown in Figure 1. Pcap parsing, packet slicing, IP modification, checksum modification, topology reconstruction, and code replay are included in the traffic replay.

  - Pcap parsing is responsible for removing useless or unrepresentative link packets and leaving *N* links to obtain the source IP list and destination IP list, after the collected normal traffic and attack traffic packet files are analyzed separately.
  - Slicing packets means removing packets whose source or destination IPs are not in the IP list parsed in the first step. Modify the source and destination IPs to Mininet intranet IPs respectively. And modifying the checksum so that the packets can be replayed.
  - Generate the corresponding Mininet topology and code for replaying traffic based on the source and destination IP lists. Running the simulated real network environment on each of the two high-performance hosts.
  - Replicating the simulated real traffic from the two high-performance hosts to the traffic analysis server after configuring the flow tables.
  - The traffic analysis server runs SDS_Project to store the extracted traffic features in the Redis database.  As a high-speed in-memory database, Redis satisfies real-time, highly concurrent requirements and is suitable for high-speed network traffic feature access.

  ![system topology](/images/system.png)

  ​																		Figure 1 System topology

  |                         Device Name                          |                     Device Configuration                     |
  | :----------------------------------------------------------: | :----------------------------------------------------------: |
  | Host1 for Replay Traffic<br>Host2 for Replay Traffic<br>Traffic Analysis System | CPU: Inter Core i7-12700K 3.6GHz<br>RAM: 128GB 3200MHz<br>ROM: 5TB<br>NIC1: 10000Mbps<br>NIC2: 1000Mbps |
  |                          TAP Switch                          |                    Centec V580-48X6Q-TAP                     |

  

  

  ​          If you have any problems, please contact zlshao@seu.edu.cn.

  
