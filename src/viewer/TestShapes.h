#pragma once

#include <TopoDS_Shape.hxx>
#include <QVariant>

// Вспомогательные функции для создания тестовых фигур
namespace TestShapes
{
  // Создание простого куба для тестирования
  TopoDS_Shape createTestBox();
  
  // Создание цилиндра для тестирования
  TopoDS_Shape createTestCylinder();
  
  // Создание сферы для тестирования
  TopoDS_Shape createTestSphere();
}
