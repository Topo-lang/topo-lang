// topo-lang — EmitterFactory contract tests

#include "MinimalPlugin.h"
#include "topo/Lang/EmitterFactory.h"
#include "topo/Transpile/Emitter.h"
#include "topo/Transpile/TranspileModel.h"

#include <gtest/gtest.h>
#include <memory>

using example::MinimalEmitterFactory;
using topo::lang::EmitterFactory;
using topo::transpile::Emitter;
using topo::transpile::TranspileModule;

TEST(EmitterFactoryTest, CreateEmitterReturnsNonNull) {
    MinimalEmitterFactory factory;
    std::unique_ptr<Emitter> emitter = factory.createEmitter();
    EXPECT_NE(emitter, nullptr);
}

TEST(EmitterFactoryTest, FileExtensionIsTxtForMinimal) {
    MinimalEmitterFactory factory;
    EXPECT_EQ(factory.fileExtension(), ".txt");
}

TEST(EmitterFactoryTest, FileExtensionStartsWithDot) {
    MinimalEmitterFactory factory;
    std::string ext = factory.fileExtension();
    ASSERT_FALSE(ext.empty());
    EXPECT_EQ(ext.front(), '.');
}

TEST(EmitterFactoryTest, CreatedEmitterProducesExpectedOutput) {
    MinimalEmitterFactory factory;
    auto emitter = factory.createEmitter();
    ASSERT_NE(emitter, nullptr);

    TranspileModule empty;
    auto output = emitter->emit(empty);
    EXPECT_EQ(output.code, "// minimal plugin output\n");
}

TEST(EmitterFactoryTest, CreateEmitterReturnsIndependentInstances) {
    MinimalEmitterFactory factory;
    auto a = factory.createEmitter();
    auto b = factory.createEmitter();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_NE(a.get(), b.get());
}

TEST(EmitterFactoryTest, EmitterFactoryInterfaceUsableViaBasePointer) {
    MinimalEmitterFactory concrete;
    EmitterFactory* base = &concrete;
    auto emitter = base->createEmitter();
    EXPECT_NE(emitter, nullptr);
    EXPECT_EQ(base->fileExtension(), ".txt");
}
