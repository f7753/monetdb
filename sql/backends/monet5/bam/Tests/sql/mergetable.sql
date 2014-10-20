SET SCHEMA bam;

# Load files
CALL bam_loader_repos('PWD/files', 0, 4);

# Add a merge table over these files
CREATE MERGE TABLE alignments (
	"virtual_offset" BIGINT        NOT NULL,
	"qname"          STRING 	   NOT NULL,
	"flag"           SMALLINT      NOT NULL,
	"rname"          STRING		   NOT NULL,
	"pos"            INT           NOT NULL,
	"mapq"           SMALLINT      NOT NULL,
	"cigar"          STRING		   NOT NULL,
	"rnext"          STRING		   NOT NULL,
	"pnext"          INT           NOT NULL,
	"tlen"           INT           NOT NULL,
	"seq"            STRING		   NOT NULL,
	"qual"           STRING		   NOT NULL
);

SELECT COUNT(*) FROM alignments;

ALTER TABLE alignments ADD TABLE alignments_1;

SELECT COUNT(*) FROM alignments;

ALTER TABLE alignments ADD TABLE alignments_2;

SELECT COUNT(*) FROM alignments;
