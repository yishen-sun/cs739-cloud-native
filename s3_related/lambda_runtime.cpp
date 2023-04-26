#include <aws/lambda-runtime/runtime.h>

#include "key_value_storage.h"  // Assuming your KeyValueStorage class is in this header file

using namespace aws::lambda_runtime;

invocation_response my_handler(invocation_request const& request) {
    Aws::String result = handle_event(request.payload);
    return invocation_response::success(result, "application/json");
}

int main() {
    run_handler(my_handler);
    return 0;
}
