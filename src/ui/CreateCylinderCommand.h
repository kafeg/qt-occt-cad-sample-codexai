#pragma once

#include "AbstractCommand.h"

class CreateCylinderCommand : public AbstractCommand
{
public:
  CreateCylinderCommand(double radius, double height)
      : myRadius(radius), myHeight(height) {}

  void execute(Document& doc) override;

private:
  double myRadius{10.0};
  double myHeight{20.0};
};
