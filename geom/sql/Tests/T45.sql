SELECT Relate(forests.boundary, named_places.boundary, 'TTTTTTTTT')
FROM forests, named_places
WHERE forests.name = 'Green Forest'
AND named_places.name = 'Ashton';
