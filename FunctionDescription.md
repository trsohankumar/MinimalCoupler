## Function Description

### Construction and Configuration

#### Signature 
```c++
precice::Participant::Participant 	( 	::precice::string_view 	participantName,
		::precice::string_view 	configurationFileName,
		int 	solverProcessIndex,
		int 	solverProcessSize )
```

#### Parameters
- [in] participantName	                : Name of the participant using the interface. 
- [in] (optional) configurationFileName	: Name (with path) of the xml configuration file.
- [in] (optional) solverProcessIndex	: If the solver code runs with several processes, each process using preCICE has to specify its index, which has to start from 0 and end with solverProcessSize - 1.
- [in] (optional) solverProcessSize	    : The number of solver processes using preCICE. 

#### Description
The participant function is responsible to construct a Participant. After construction the participant has knowledge about:
- Its own identity that is who it is, its rank and the number of solver process.
- The meshes it provides or recieves and their associated data fields.
- Its coupling scheme partners and the data exchange relationship between them.
- The coupling scheme parameters like max coupling time and time window size.
The method of retreiving this information and constructing the participant is left to the implementor.

### Status Queries

#### Signature

```c++
int precice::Participant::getMeshDimensions (::precice::string_view meshName) 	const
```

#### Parameters

[in] meshName : The name used to identify the mesh whose dimension is to be retreived.

#### Description
The function is responsible to return the dimensionality of the mesh that is identified by meshName.


### Mesh access

#### Signature
```c++
void precice::Participant::setMeshVertices(
    ::precice::string_view 	meshName,
    ::precice::span <const double> coordinates,
	::precice::span<VertexID> ids )
```

#### Parameters
- [in]	meshName: The name used to identify the mesh whose vertices are to be added.
- [in] coordinates: It is a flat array of coordinates. For an n dimensional point, n consecutive values form a single coordinate.
- [out] ids	: An array of identifiers used to refer to individual coordinates.

#### Description
- The function is responsible to add vertices to  a mesh. After adding vertices, the mesh identified by meshName must have:
- Coordinates that form the mesh points.
- Unique identifiers to identify each of the added coordinates stored in ids.
-  Data buffers allocated for all the added vertices.