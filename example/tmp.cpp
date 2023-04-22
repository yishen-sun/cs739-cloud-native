#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <iostream>
#include <fstream>

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

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    try {
        Aws::Client::ClientConfiguration client_config;
        client_config.region = "us-east-2";
        Aws::S3::S3Client s3_client(client_config);
        // Set your bucket name and prefix (if any) 
        const Aws::String bucket_name = "bucketencoder";
        const Aws::String prefix = "test/";

        // List all objects in the bucket with the specified prefix
        Aws::S3::Model::ListObjectsRequest list_objects_request;
        list_objects_request.SetBucket(bucket_name);
        list_objects_request.SetPrefix(prefix);

        auto list_objects_outcome = s3_client.ListObjects(list_objects_request);

        if (list_objects_outcome.IsSuccess()) {
            for (const auto& object : list_objects_outcome.GetResult().GetContents()) {
                // Download the object to a local file
                
                Aws::String key = object.GetKey();
                std::cout << "key: " << key << std::endl;
                std::string local_file_path = key;//.substr(prefix.size());
                std::string is_dir = key.substr(prefix.size());
                if (is_dir.size() == 0) continue;
                Aws::S3::Model::GetObjectRequest get_object_request;
                get_object_request.SetBucket(bucket_name);
                get_object_request.SetKey(key);

                auto get_object_outcome = s3_client.GetObject(get_object_request);
                if (get_object_outcome.IsSuccess()) {
                    std::ofstream output_file(local_file_path, std::ios::binary);
                    output_file << get_object_outcome.GetResult().GetBody().rdbuf();
                } else {
                    std::cerr << "Error downloading " << key << ": " << get_object_outcome.GetError().GetMessage() << std::endl;
                }

                // Modify the local file
                ModifyFile(local_file_path);

                // Upload the modified file back to the S3 bucket
                Aws::S3::Model::PutObjectRequest put_object_request;
                put_object_request.SetBucket(bucket_name);
                put_object_request.SetKey(key);

                std::shared_ptr<Aws::IOStream> input_data = std::make_shared<Aws::FStream>(local_file_path.c_str(), std::ios_base::in | std::ios_base::binary);
                put_object_request.SetBody(input_data);

                auto put_object_outcome = s3_client.PutObject(put_object_request);
                if (!put_object_outcome.IsSuccess()) {
                    std::cerr << "Error uploading " << key << ": " << put_object_outcome.GetError().GetMessage() << std::endl;
                }
            }
        } else {
            std::cerr << "Error listing objects: " << list_objects_outcome.GetError().GetMessage() << std::endl;
        }



    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
    }
        
    Aws::ShutdownAPI(options);
    return 0;
}
    
