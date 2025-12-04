// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2015, 2016, 2017 Neuro-Electronics Research Flanders
//
// Falcon-server is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falcon-server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with falcon-core. If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------
#pragma once

#include "datatype1/datatype1.hpp"
#include "datatype1/datatype2.hpp"
#include "iprocessor.hpp"

class FalconProcessor : public IProcessor {
  public:
    void Configure(const GlobalContext &context) override;
    void CreatePorts() override;
    void CompleteStreamInfo() override;
    void Prepare(GlobalContext &context) override;
    void Preprocess(ProcessingContext &context) override;
    void Process(ProcessingContext &context) override;
    void Postprocess(ProcessingContext &context) override;
    void Unprepare(GlobalContext &context) override;

  protected:
    void method1();

  protected:
    PortIn<AnyDataType> *data_in_port_;
    PortIn<AnyDataType> *other_data_in_port_;
    PortOut<AnyDataType> *data_out_port_;
    PortOut<AnyDataType> *other_data_out_port_;

    WritableState<T1> *state_variable1_;
    ReadableState<T2> *state_variable2_;

    T1 default_state_variable1_;
    T2 default_state_variable2_;

    datatype1 option1_;
    datatype2 option2_;

  public:
    const datatype1 DEFAULT_OPTION1 = 6;
    static constexpr datatype DEFAULT_ARGUMENT3 = 50;
    static constexpr datatype DEFAULT_ARGUMENT4 = DEFAULT_ARGUMENT3 * 2;
};
