#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

int ModifyFile(const std::string& file_path) {
    std::cout << file_path << std::endl;
    // Implement your file modification logic here
    std::string data_to_append = "This is the data to append.";

    std::fstream file(file_path, std::ios::in | std::ios::out | std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    // Read all the data from the file
    std::string file_contents;
    std::string line;
    while (std::getline(file, line)) {
        file_contents += line + '\n';
    }

    // Print the file contents
    std::cout << "File contents before appending:" << std::endl;
    std::cout << file_contents << std::endl;

    file.close();
    std::fstream file1(file_path, std::ios::in | std::ios::out | std::ios::app);
    // Append data to the file
    file1 << data_to_append;

    // Close the file

    file1.close();
    return 0;

}


class s3Manager {
  public:
    s3Manager(const std::string &bucket_name, const std::string &prefix, const std::string &region):
            bucket_name(bucket_name), 
            prefix(prefix),
            options(Aws::SDKOptions())
            {
                std::cout << "constructor called" << std::endl;
                Aws::InitAPI(options);
                std::cout << "1" << std::endl;
                client_config = new Aws::Client::ClientConfiguration();
                //client_config = std::make_unique<Aws::Client::ClientConfiguration>();
                std::cout << "2" << std::endl;
                client_config->region = region;
                std::cout << "3" << std::endl;
                s3_client = new Aws::S3::S3Client(*client_config);
                //s3_client = std::make_unique<Aws::S3::S3Client>(*client_config);
                std::cout << "4" << std::endl;
        
        

    }
    ~s3Manager() {
        std::cout << "destructor called" << std::endl;
        delete client_config;
        std::cout << "5" << std::endl;
        delete s3_client;
        std::cout << "6" << std::endl;
        Aws::ShutdownAPI(options);
        std::cout << "7" << std::endl;
    }
    int download() {
        Aws::S3::Model::ListObjectsRequest list_objects_request;
        list_objects_request.SetBucket(bucket_name);
        list_objects_request.SetPrefix(prefix);
        auto list_objects_outcome = s3_client->ListObjects(list_objects_request);
        if (!list_objects_outcome.IsSuccess()) {
            std::cerr << "Error listing objects: " << list_objects_outcome.GetError().GetMessage() << std::endl;
            return 1;
        }
        for (const auto& object : list_objects_outcome.GetResult().GetContents()) {
            // Download the object to a local file
            Aws::String key = object.GetKey();
            std::cout << "key: " << key << std::endl;
            std::string local_file_path = key;
            std::string is_dir = key.substr(prefix.size());
            if (is_dir.size() == 0) continue;
            Aws::S3::Model::GetObjectRequest get_object_request;
            get_object_request.SetBucket(bucket_name);
            get_object_request.SetKey(key);

            auto get_object_outcome = s3_client->GetObject(get_object_request);
            if (get_object_outcome.IsSuccess()) {
                std::ofstream output_file(local_file_path, std::ios::binary);
                output_file << get_object_outcome.GetResult().GetBody().rdbuf();
            } else {
                std::cerr << "Error downloading " << key << ": " << get_object_outcome.GetError().GetMessage() << std::endl;
                return 1;
            }
            //ModifyFile(local_file_path);
        }
        return 0;
    }
    int upload() {
        std::vector<std::string> file_names;

        // Iterate through the directory entries
        for (const auto& entry : std::filesystem::directory_iterator(prefix)) {
            // Check if the entry is a regular file
            if (entry.is_regular_file()) {
                // Add the file name to the vector
                file_names.push_back(prefix + entry.path().filename().string());
            }
        }
        for (auto key : file_names) {
            Aws::S3::Model::PutObjectRequest put_object_request;
            put_object_request.SetBucket(bucket_name);
            put_object_request.SetKey(key);

            std::shared_ptr<Aws::IOStream> input_data = std::make_shared<Aws::FStream>(key.c_str(), std::ios_base::in | std::ios_base::binary);
            put_object_request.SetBody(input_data);

            auto put_object_outcome = s3_client->PutObject(put_object_request);
            if (!put_object_outcome.IsSuccess()) {
                std::cerr << "Error uploading " << key << ": " << put_object_outcome.GetError().GetMessage() << std::endl;
                return 1;
            }
        }
        return 0;     
    }

  private:
    
    Aws::String bucket_name;
    Aws::String prefix;
    Aws::SDKOptions options;
    Aws::Client::ClientConfiguration* client_config;
    Aws::S3::S3Client* s3_client;
};