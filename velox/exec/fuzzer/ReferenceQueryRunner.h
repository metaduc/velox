/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <set>
#include "velox/core/PlanNode.h"
#include "velox/vector/fuzzer/VectorFuzzer.h"

namespace facebook::velox::exec::test {

/// Query runner that uses reference database, i.e. DuckDB, Presto, Spark.
class ReferenceQueryRunner {
 public:
  enum class RunnerType {
    kPrestoQueryRunner,
    kDuckQueryRunner,
    kSparkQueryRunner
  };

  // @param aggregatePool Used to allocate memory needed for vectors produced by
  // 'execute' methods.
  explicit ReferenceQueryRunner(memory::MemoryPool* aggregatePool)
      : aggregatePool_(aggregatePool) {}

  virtual ~ReferenceQueryRunner() = default;

  virtual RunnerType runnerType() const = 0;

  // Scalar types supported by the reference database, to be used to restrict
  // candidates when generating random types for fuzzers.
  virtual const std::vector<TypePtr>& supportedScalarTypes() const {
    return defaultScalarTypes();
  }

  virtual const std::unordered_map<std::string, DataSpec>&
  aggregationFunctionDataSpecs() const = 0;

  /// Converts Velox plan into SQL accepted by the reference database.
  /// @return std::nullopt if the plan uses features not supported by the
  /// reference database.
  virtual std::optional<std::string> toSql(const core::PlanNodePtr& plan) = 0;

  /// Executes SQL query returned by the 'toSql' method using 'input' data.
  /// Converts results using 'resultType' schema.
  virtual std::multiset<std::vector<velox::variant>> execute(
      const std::string& sql,
      const std::vector<RowVectorPtr>& input,
      const RowTypePtr& resultType) = 0;

  /// Executes SQL query returned by the 'toSql' method using 'probeInput' and
  /// 'buildInput' data for join node.
  /// Converts results using 'resultType' schema.
  virtual std::multiset<std::vector<velox::variant>> execute(
      const std::string& sql,
      const std::vector<RowVectorPtr>& probeInput,
      const std::vector<RowVectorPtr>& buildInput,
      const RowTypePtr& resultType) = 0;

  /// Returns true if 'executeVector' can be called to get results as Velox
  /// Vector.
  virtual bool supportsVeloxVectorResults() const {
    return false;
  }

  /// Similar to 'execute' but returns results in RowVector format.
  /// Caller should ensure 'supportsVeloxVectorResults' returns true.
  virtual std::vector<RowVectorPtr> executeVector(
      const std::string& sql,
      const std::vector<RowVectorPtr>& input,
      const RowTypePtr& resultType) {
    VELOX_UNSUPPORTED();
  }

  /// Similar to above but for join node with 'probeInput' and 'buildInput'.
  virtual std::vector<RowVectorPtr> executeVector(
      const std::string& sql,
      const std::vector<RowVectorPtr>& probeInput,
      const std::vector<RowVectorPtr>& buildInput,
      const RowTypePtr& resultType) {
    VELOX_UNSUPPORTED();
  }

  virtual std::vector<velox::RowVectorPtr> execute(const std::string& sql) {
    VELOX_UNSUPPORTED();
  }

  virtual std::vector<velox::RowVectorPtr> execute(
      const std::string& sql,
      const std::string& sessionProperty) {
    VELOX_UNSUPPORTED();
  }

 protected:
  memory::MemoryPool* aggregatePool() {
    return aggregatePool_;
  }

 private:
  memory::MemoryPool* aggregatePool_;
};

} // namespace facebook::velox::exec::test
