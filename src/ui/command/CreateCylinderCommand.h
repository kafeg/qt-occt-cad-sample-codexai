#pragma once

#include "AbstractCommand.h"

// Command: create a cylinder Feature and append it to a Document
class CreateCylinderCommand : public AbstractCommand
{
public:
  CreateCylinderCommand(double radius, double height)
      : m_radius(radius), m_height(height) {}

  void execute(Document& doc) override; // pushes feature and triggers recompute

private:
  double m_radius{10.0}; // radius
  double m_height{20.0}; // height
};

