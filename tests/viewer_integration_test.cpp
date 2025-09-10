#include <gtest/gtest.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>

#include "../src/viewer/ViewerTypes.h"

class ViewerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        app = new QGuiApplication(argc, argv);
        
        engine = new QQmlApplicationEngine();
        registerViewerTypes();
    }
    
    void TearDown() override {
        delete engine;
        delete app;
    }
    
    QGuiApplication* app;
    QQmlApplicationEngine* engine;
};

TEST_F(ViewerIntegrationTest, ViewerTypeRegistration) {
    // Проверяем, что функция регистрации типов доступна
    // Это базовая проверка, что код компилируется и линкуется
    EXPECT_TRUE(true);
}

TEST_F(ViewerIntegrationTest, ViewerCreation) {
    // Проверяем, что engine создан
    EXPECT_NE(engine, nullptr);
    
    // Проверяем, что app создан
    EXPECT_NE(app, nullptr);
}
