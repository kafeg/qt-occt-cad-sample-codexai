#pragma once

#include "AbstractCommand.h"

class CreateBoxCommand : public AbstractCommand
{
public:
  CreateBoxCommand(double dx, double dy, double dz)
      : m_dx(dx),
        m_dy(dy),
        m_dz(dz)
  {
  }

  void execute(Document& doc) override;

private:
  double m_dx{10.0};
  double m_dy{10.0};
  double m_dz{10.0};
};
