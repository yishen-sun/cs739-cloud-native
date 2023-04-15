#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include <iostream>
#include <sstream>

class KeyValueStorage {
   public:
    KeyValueStorage() {
        Aws::SDKOptions options;
        Aws::InitAPI(options);
        client = std::make_shared<Aws::S3::S3Client>();
        bucket_name = "my-key-value-bucket";
    }

    ~KeyValueStorage() {
        Aws::SDKOptions options;
        Aws::ShutdownAPI(options);
    }

    bool insert(const std::string& key, const std::string& value) {
        Aws::S3::Model::PutObjectRequest req;
        req.SetBucket(bucket_name);
        req.SetKey(key);
        auto input_data = std::make_shared<std::stringstream>();
        *input_data << value;
        req.SetBody(input_data);
        auto outcome = client->PutObject(req);
        return outcome.IsSuccess();
    }

    std::string get(const std::string& key) {
        Aws::S3::Model::GetObjectRequest req;
        req.SetBucket(bucket_name);
        req.SetKey(key);
        auto outcome = client->GetObject(req);
        if (outcome.IsSuccess()) {
            std::stringstream result;
            result << outcome.GetResult().GetBody().rdbuf();
            return result.str();
        }
        return "";
    }

    bool update(const std::string& key, const std::string& value) { return insert(key, value); }

    bool remove(const std::string& key) {
        Aws::S3::Model::DeleteObjectRequest req;
        req.SetBucket(bucket_name);
        req.SetKey(key);
        auto outcome = client->DeleteObject(req);
        return outcome.IsSuccess();
    }

   private:
    std::shared_ptr<Aws::S3::S3Client> client;
    Aws::String bucket_name;
};
