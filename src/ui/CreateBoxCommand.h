#pragma once

#include "AbstractCommand.h"

class CreateBoxCommand : public AbstractCommand
{
public:
  CreateBoxCommand(double dx, double dy, double dz)
    : myDx(dx), myDy(dy), myDz(dz) {}

  void execute(Document& doc) override;

private:
  double myDx{10.0};
  double myDy{10.0};
  double myDz{10.0};
};

