#include <iostream>
#include <sstream>
#include <vector>
#include "precice/precice.hpp"

int main(int argc, char **argv)
{
  int commRank = 0;
  int commSize = 1;

  using namespace precice;

  if (argc != 2) {
    std::cout << "The solverdummy was called with an incorrect number of arguments. Usage: ./solverdummy solverName\n\n";
    std::cout << "Parameter description\n";
    std::cout << "solverName: SolverDummy participant name\n";
    return EXIT_FAILURE;
  }

  std::string solverName(argv[1]);
  std::string meshName;
  std::string dataWriteName;
  std::string dataReadName;
  std::string configFileName{""};

  Participant participant(solverName, configFileName, commRank, commSize);

  if (solverName == "Fluid") {
    dataWriteName = "Force";
    dataReadName  = "Displacement";
    meshName      = "Fluid-Mesh";
  }
  if (solverName == "Solid") {
    dataReadName  = "Force";
    dataWriteName = "Displacement";
    meshName      = "Solid-Mesh";
  }

  int dimensions       = participant.getMeshDimensions(meshName);
  int numberOfVertices = 3;

  std::vector<double> vertices(numberOfVertices * dimensions);
  std::vector<int>    vertexIDs(numberOfVertices);

  for (int i = 0; i < numberOfVertices; i++) {
    for (int j = 0; j < dimensions; j++) {
      vertices.at(j + dimensions * i) = i;
    }
  }

  participant.setMeshVertices(meshName, vertices, vertexIDs);

  std::vector<double> readData(numberOfVertices * dimensions);
  std::vector<double> writeData(numberOfVertices * dimensions);
  for (int i = 0; i < numberOfVertices; i++) {
    for (int j = 0; j < dimensions; j++) {
      readData.at(j + dimensions * i)  = i;
      writeData.at(j + dimensions * i) = i;
    }
  }

  if (participant.requiresInitialData()) {
    std::cout << "DUMMY: Writing initial data\n";
  }

  participant.initialize();

  while (participant.isCouplingOngoing()) {

    if (participant.requiresWritingCheckpoint()) {
      std::cout << "DUMMY: Writing iteration checkpoint\n";
    }

    double dt = participant.getMaxTimeStepSize();
    participant.readData(meshName, dataReadName, vertexIDs, dt, readData);

    for (int i = 0; i < numberOfVertices * dimensions; i++) {
      writeData.at(i) = readData.at(i) + 1;
    }

    participant.writeData(meshName, dataWriteName, vertexIDs, writeData);

    participant.advance(dt);

    if (participant.requiresReadingCheckpoint()) {
      std::cout << "DUMMY: Reading iteration checkpoint\n";
    } else {
      std::cout << "DUMMY: Advancing in time\n";
    }
  }

  participant.finalize();
  std::cout << "DUMMY: Closing C++ solver dummy...\n";

  return 0;
}
