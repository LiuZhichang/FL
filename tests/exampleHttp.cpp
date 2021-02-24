#include "../src/FL/http/http.h"

void test_requese() {

    FL::http::HttpRequest::ptr req(new FL::http::HttpRequest);
    req->setHeader("host", "www.fl.top");
    req->setBody("hello fl");

    req->dump(std::cout) << std::endl;
}

void test_response() {
    FL::http::HttpResponse::ptr rsp(new FL::http::HttpResponse);
    rsp->setHeader("x-x", "fl");
    rsp->setBody("hello fl");
	rsp->setStatus((FL::http::HttpStatus)400);
	rsp->setClose(false);
    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {

    test_requese();
    std::cout << std::endl;
    test_response();

    return 0;
}
