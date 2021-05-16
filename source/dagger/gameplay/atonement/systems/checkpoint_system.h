#pragma once

#include "core/system.h"
#include "core/core.h"
#include <vector>

using namespace dagger;

namespace atonement {

    class CheckpointSystem : public System
    {

    public:
        inline String SystemName() { return "Checkpoint System"; }

        void SpinUp() override;
        void Run() override;
        void WindDown() override;

        void RepositionPlayer();

    private:
        Vector3 currentCheckpoint = {0, 0, 0};
        std::vector<Vector3> possibleCheckpoints = { Vector3{4190, 700, 15},
                                                     Vector3{6000, 800, 15} };
    };
}