-- Conformance Item T41
SELECT Overlaps(forests.boundary, named_places.boundary) FROM forests, named_places WHERE forests.name = 'Green Forest' AND named_places.name = 'Ashton';
