#pragma once

#include "AbstractCommand.h"
#include <memory>

#include <Standard_DefineHandle.hxx>

class Sketch;
class PlaneFeature;

// Command: create a new Sketch bound to a PlaneFeature (by id and Ax2)
class CreateSketchCommand : public AbstractCommand
{
public:
  explicit CreateSketchCommand(const Handle(PlaneFeature)& plane)
    : m_plane(plane) {}

  void execute(Document& doc) override; // creates and registers a Sketch, no recompute needed

  std::shared_ptr<Sketch> createdSketch() const { return m_sketch; }

private:
  Handle(PlaneFeature)      m_plane;   // reference plane
  std::shared_ptr<Sketch>   m_sketch;  // result after execute
};

