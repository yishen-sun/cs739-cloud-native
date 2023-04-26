#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

Aws::String handle_event(const Aws::String& event) {
    // Parse input event (JSON format)
    Aws::Utils::Json::JsonValue json_event(event);
    Aws::String action = json_event.GetString("action");
    Aws::String key = json_event.GetString("key");
    Aws::String value = json_event.GetString("value");

    // Perform the corresponding action
    KeyValueStorage kv_storage;
    Aws::String result;
    if (action == "insert") {
        if (kv_storage.insert(key, value)) {
            result = "Insertion successful.";
        } else {
            result = "Insertion failed.";
        }
    } else if (action == "get") {
        result = kv_storage.get(key);
    } else if (action == "update") {
        if (kv_storage.update(key, value)) {
            result = "Update successful.";
        } else {
            result = "Update failed.";
        }
    } else if (action == "remove") {
        if (kv_storage.remove(key)) {
            result = "Deletion successful.";
        } else {
            result = "Deletion failed.";
        }
    } else {
        result = "Invalid action.";
    }

    return result;
}
