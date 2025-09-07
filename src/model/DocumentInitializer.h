#pragma once

#include <memory>

class Document;

// Helper to populate a Document with default datum-driven geometry
// - Inserts XY, XZ, YZ PlaneFeature items using the document's Datum parameters
// - Inserts X, Y, Z AxeFeature items representing datum axes
// - Optionally inserts a PointFeature at the origin when Datum::showOriginPoint() is true
namespace DocumentInitializer
{
void initialize(Document& doc);
}
