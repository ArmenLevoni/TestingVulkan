//
//  ComputeMain.cpp
//  TestingVulkan
//
//  Created by Armen Khachatryan on 20/08/2019.
//  Copyright © 2019 Armen Khachatryan. All rights reserved.
//

#include "ComputeMain.hpp"
#include "BaseApp.hpp"
#include <iostream>

class ComputeMain : public BaseApp {

public:
    void run () {
        initVulkan();
    }
    
};


int main() {
    
    ComputeMain app;
    
    try {
        app.inBufferSize = sizeof(BaseApp::duble_fe25519) * WORK_TOTAL_SIZE;
        app.outBufferSize = sizeof(BaseApp::fe25519) * WORK_TOTAL_SIZE;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
