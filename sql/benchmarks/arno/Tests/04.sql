SELECT MODEL2.is_mutagen,MODEL2.is_mutagen, count(distinct MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by MODEL2.is_mutagen , MODEL2.is_mutagen;
