#pragma once

#include "AbstractCommand.h"
#include <memory>

class Sketch;

// Command: create an ExtrudeFeature from a Sketch and append it to a Document
class CreateExtrudeCommand : public AbstractCommand
{
public:
  CreateExtrudeCommand(std::shared_ptr<Sketch> sketch, double distance)
      : m_sketch(std::move(sketch)), m_distance(distance) {}

  void execute(Document& doc) override; // pushes feature and triggers recompute

private:
  std::shared_ptr<Sketch> m_sketch; // profile sketch
  double                  m_distance{10.0}; // extrusion length along +Z
};

