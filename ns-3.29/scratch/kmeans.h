#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
class psnr{
public:
    int NodeID;
    vector<double> values;
    int ClusterID;
    int IsClient;
    int getNodeID()
    {
        return NodeID;
    }
    int getClusterID()
    {
        return ClusterID;
    }
    int getIsClient()
    {
        return IsClient;
    }
    psnr(string line)
   {
        stringstream is(line);
        double val;
        while(is>>val)
        {
            values.push_back(val);
        }
        ClusterID=0;
    } 
};

class Point{

public:
    int pointId, clusterId;
    int dimensions;
    vector<double> values;

public:
    Point(int id, string line){
        dimensions = 0;
        pointId = id;
        stringstream is(line);
        double val;
        while(is >> val){
            values.push_back(val);
            dimensions++;
        }
        clusterId = 0; //Initially not assigned to any cluster
    }

    int getDimensions(){
        return dimensions;
    }

    int getCluster(){
        return clusterId;
    }

    int getID(){
        return pointId;
    }

    void setCluster(int val){
        clusterId = val;
    }

    double getVal(int pos){
        return values[pos];
    }
};

class Cluster{

public:
    int clusterId;
    vector<double> centroid;
    vector<Point> points;

public:
    Cluster(int clusterId, Point centroid){
        this->clusterId = clusterId;
        for(int i=0; i<centroid.getDimensions(); i++){
            this->centroid.push_back(centroid.getVal(i));
        }
        this->addPoint(centroid);
    }

    void addPoint(Point p){
        p.setCluster(this->clusterId);
        points.push_back(p);
    }

    bool removePoint(int pointId){
        int size = points.size();

        for(int i = 0; i < size; i++)
        {
            if(points[i].getID() == pointId)
            {
                points.erase(points.begin() + i);
                return true;
            }
        }
        return false;
    }

    int getId(){
        return clusterId;
    }

    Point getPoint(int pos){
        return points[pos];
    }

    int getSize(){
        return points.size();
    }

    double getCentroidByPos(int pos) {
        return centroid[pos];
    }

    void setCentroidByPos(int pos, double val){
        this->centroid[pos] = val;
    }
};

class KMeans{
public:
    int K, iters, dimensions, total_points;
    vector<Cluster> clusters;

    int getNearestClusterId(Point point){
        double sum = 0.0, min_dist;
        int NearestClusterId;

        for(int i = 0; i < dimensions; i++)
        {
            sum += pow(clusters[0].getCentroidByPos(i) - point.getVal(i), 2.0);
        }

        min_dist = sqrt(sum);
        NearestClusterId = clusters[0].getId();

        for(int i = 1; i < K; i++)
        {
            double dist;
            sum = 0.0;

            for(int j = 0; j < dimensions; j++)
            {
                sum += pow(clusters[i].getCentroidByPos(j) - point.getVal(j), 2.0);
            }

            dist = sqrt(sum);

            if(dist < min_dist)
            {
                min_dist = dist;
                NearestClusterId = clusters[i].getId();
            }
        }

        return NearestClusterId;
    }

public:
    KMeans(int K, int iterations){
        this->K = K;
        this->iters = iterations;
    }

    void run(vector<Point>& all_points){

        total_points = all_points.size();
        dimensions = all_points[0].getDimensions();


        //Initializing Clusters
        vector<int> used_pointIds;

        for(int i=1; i<=K; i++)
        {
            while(true)
            {
                int index = rand() % total_points;

                if(find(used_pointIds.begin(), used_pointIds.end(), index) == used_pointIds.end())
                {
                    used_pointIds.push_back(index);
                    all_points[index].setCluster(i);
                    Cluster cluster(i, all_points[index]);
                    clusters.push_back(cluster);
                    break;
                }
            }
        }
        cout<<"Clusters initialized = "<<clusters.size()<<endl<<endl;


        cout<<"Running K-Means Clustering for Sharding"<<endl;

        int iter = 1;
        while(true)
        {
            cout<<"Iter - "<<iter<<"/"<<iters<<endl;
            bool done = true;

            // Add all points to their nearest cluster
            for(int i = 0; i < total_points; i++)
            {
                int currentClusterId = all_points[i].getCluster();
                int nearestClusterId = getNearestClusterId(all_points[i]);

                if(currentClusterId != nearestClusterId)
                {
                    if(currentClusterId != 0){
                        for(int j=0; j<K; j++){
                            if(clusters[j].getId() == currentClusterId){
                                clusters[j].removePoint(all_points[i].getID());
                            }
                        }
                    }

                    for(int j=0; j<K; j++){
                        if(clusters[j].getId() == nearestClusterId){
                            clusters[j].addPoint(all_points[i]);
                        }
                    }
                    all_points[i].setCluster(nearestClusterId);
                    done = false;
                }
            }

            // Recalculating the center of each cluster
            for(int i = 0; i < K; i++)
            {
                int ClusterSize = clusters[i].getSize();

                for(int j = 0; j < dimensions; j++)
                {
                    double sum = 0.0;
                    if(ClusterSize > 0)
                    {
                        for(int p = 0; p < ClusterSize; p++)
                            sum += clusters[i].getPoint(p).getVal(j);
                        clusters[i].setCentroidByPos(j, sum / ClusterSize);
                    }
                }
            }

            if(done || iter >= iters)
            {
                cout << "Clustering completed in iteration : " <<iter<<endl<<endl;
                break;
            }
            iter++;
        }


        //Print pointIds in each cluster

        ofstream shardfile;
        shardfile.open("ShardFile.txt");
        shardfile<<"Node ID"<<" coordinates"<<" Cluster ID"<<" IsClient"<<endl;
        for(int i=0; i<K; i++){
            cout<<"________Coordinates of nodes in Shard "<<clusters[i].getId()<<"_______"<<endl;
            int flag=1;
            for(int j=0; j<clusters[i].getSize(); j++){
                int k=clusters[i].getPoint(j).getID()-1;
                cout<<k<<" : ";
                cout<<"("<<all_points[k].values[0]<<","<<all_points[k].values[1]<<","<<all_points[k].values[2]<<")"<<endl;
                shardfile<<k<<" "<<all_points[k].values[0]<<" "<<all_points[k].values[1]<<" "<<all_points[k].values[2]<<" "<<i<<" ";
                if(flag==0) {shardfile<<0;flag=1;}
                else {shardfile<<1;flag=0;}
                shardfile<<endl;
            }
            cout<<endl;
        }
        shardfile.close();
        cout<<endl;

        ofstream cufile;
        cufile.open("cu.txt");
        //Write cluster centers to file
        cout<<"One CU per Shard is considered"<<endl;
        if(1){
            for(int i=0; i<K; i++){
                cout<<"Shard "<<clusters[i].getId()<<" CU coordinates : ";
                for(int j=0; j<dimensions; j++){
                    cout<<clusters[i].getCentroidByPos(j)<<" ";     //Output to terminal
                    cufile<<clusters[i].getCentroidByPos(j)<<" ";
                }
                cufile<<endl;
                cout<<endl;
            }
        }
        else{
            cout<<"Error: Unable to write to clusters.txt";
        }
        cufile.close();
    }
};
/*
ostream& operator<<(ostream& os, const vector<double>& v) 
{ 
    os << "["; 
    for (int i = 0; i < v.size(); ++i) { 
        os << v[i]; 
        if (i != v.size() - 1) 
            os << ", "; 
    } 
    os << "]\n"; 
    return os; 
}*/

vector<Point> kmeans_main(int K){

    string filename = "input.txt";
    ifstream infile(filename.c_str());
    if(!infile.is_open()){
        cout<<"Error: Failed to open file."<<endl;
    }

    //Fetching points from file
    int pointId = 1;
    vector<Point> all_points;
    string line;

    while(getline(infile, line)){
        Point point(pointId, line);
        all_points.push_back(point);
        pointId++;
    }
    infile.close();
    //cout<<"\nData fetched successfully!"<<endl<<endl;

    //Running K-Means Clustering
    
    int iters = 100;

    KMeans kmeans(K, iters);
    kmeans.run(all_points);
    return all_points;
}