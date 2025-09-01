#pragma once

#include "AbstractCommand.h"

class CreateCylinderCommand : public AbstractCommand
{
public:
  CreateCylinderCommand(double radius, double height)
      : m_radius(radius), m_height(height) {}

  void execute(Document& doc) override;

private:
  double m_radius{10.0};
  double m_height{20.0};
};
