#pragma once

#include "AbstractCommand.h"

// Command: create a box Feature and append it to a Document
class CreateBoxCommand : public AbstractCommand
{
public:
  CreateBoxCommand(double dx, double dy, double dz)
      : m_dx(dx),
        m_dy(dy),
        m_dz(dz)
  {
  }

  void execute(Document& doc) override; // pushes feature and triggers recompute

private:
  double m_dx{10.0}; // X length
  double m_dy{10.0}; // Y length
  double m_dz{10.0}; // Z length
};

