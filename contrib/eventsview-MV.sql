CREATE VIEW eventsview as select cnt_useid useid, cnt_eventid eventid, cnt_channelid channelid, cnt_source source, all_updsp updsp, cnt_updflg updflg, cnt_delflg delflg, cnt_fileref fileref, cnt_tableid tableid, cnt_version version, sub_title title,
case when sub_shorttext is Null then
 concat(
  case when length(ifnull(sub_genre,'')) > 0 then sub_genre else '' end,
  case when length(ifnull(sub_genre,'')) > 0 and length(ifnull(sub_country,'')) + length(ifnull(sub_year,'')) > 0 then ' (' else '' end,
  case when length(ifnull(sub_country,'')) > 0 then sub_country else '' end,
  case when length(ifnull(sub_country,'')) > 0 and length(ifnull(sub_year,'')) > 0 then ' ' else '' end,
  case when length(ifnull(sub_year,'')) > 0 then sub_year else '' end,
  case when length(ifnull(sub_genre,'')) > 0 and length(ifnull(sub_country,'')) + length(ifnull(sub_year,'')) > 0 then ')' else '' end
 )
else
 sub_shorttext
end shorttext,
case when sub_longdescription is Null then
  cnt_longdescription
else
  sub_longdescription
end longdescription,
case when cnt_source <> sub_source then
  concat(upper(replace(cnt_source,'vdr','dvb')),'/',upper(sub_source))
else
  upper(replace(cnt_source,'vdr','dvb'))
end mergesource,
cnt_starttime starttime, cnt_duration duration, cnt_parentalrating parentalrating, cnt_vps vps, cnt_contents contents, replace(
concat(
  TRIM(LEADING '|' FROM
   concat(
    case when sub_genre is Null then '' else concat('|Genre: ', sub_genre) end,
    case when sub_category is Null then '' else concat('|Kategorie: ', sub_category) end,
    case when sub_country is Null then '' else concat('|Land: ', sub_country) end,
    case when sub_year is Null then '' else concat('|Jahr: ', substring(sub_year,1,4)) end
   )
  ),
  concat(
    case when sub_shortdescription is Null then '' else concat('||', sub_shortdescription) end,
    case when sub_txtrating is Null and sub_shortreview is Null then '' else '||' end,
    case when sub_txtrating is Null then '' else sub_txtrating end,
    case when sub_txtrating is Null or sub_shortreview is Null then '' else ', ' end,
    case when sub_shortreview is Null then '' else sub_shortreview end,
    case when sub_tipp is Null and sub_rating is Null then '' else '||' end,
    case when sub_tipp is Null then '' else concat('|', sub_tipp) end,
    case when sub_rating is Null then '' else concat('|', sub_rating) end,
    case when sub_topic is Null then '' else concat('||Thema: ', sub_topic) end,
    case when sub_longdescription is Null and cnt_longdescription is Null then '|| >>> Keine Beschreibung verfügbar! <<<'
    else
      case when sub_longdescription is Null then concat('||', cnt_longdescription, ' [DVB]')
      else
        concat('||', sub_longdescription) end end,
    case when sub_moderator is Null then '' else concat('||Moderation: ', sub_moderator) end,
    case when sub_commentator is Null then '' else concat('||Kommentar: ', sub_commentator) end,
    case when sub_guest is Null then '' else concat('|Gäste: ', sub_guest) end,
    case when cnt_parentalrating is Null or cnt_parentalrating = 0 then '' else concat('||Altersempfehlung: ab ', cnt_parentalrating) end,
    case when sub_actor is Null and sub_producer is Null and sub_other is Null then '' else '|' end,
    /*case when sub_actor is Null then '' else concat('|Darsteller: ', replace(sub_actor, '\n/\n', '/')) end,*/
    case when sub_actor is Null then '' else concat('|Darsteller: ', sub_actor) end,
    case when sub_producer is Null then '' else concat('|Produktion: ', sub_producer) end,
    case when sub_other is Null then '' else concat('|Sonstige: ', sub_other) end,
    case when sub_director is Null and sub_screenplay is Null and sub_camera is Null and sub_music is Null and sub_audio is Null and sub_flags is Null then '' else '|' end,
    case when sub_director is Null then '' else concat('|Regie: ', sub_director) end,
    case when sub_screenplay is Null then '' else concat('|Drehbuch: ', sub_screenplay) end,
    case when sub_camera is Null then '' else concat('|Kamera: ', sub_camera) end,
    case when sub_music is Null then '' else concat('|Musik: ', sub_music) end,
    case when sub_audio is Null then '' else concat('|Audio: ', sub_audio) end,
    case when sub_flags is Null then '' else concat('|Flags: ', sub_flags) end,
    case when epi_episodename is Null then '' else concat('||Serie: ', epi_episodename) end,
    /* Kurzname der Serie */
    case when epi_shortname is Null then '' else concat('|Kurzname: ', epi_shortname) end,
    /*
    case when epi_shortname is Null then ''
    else
      case when epi_shortname <> epi_episodename then concat('|Kurzname: ', epi_shortname)
      else
        '' end end,
    */
    case when epi_extracol1 is Null then '' else concat('|', epi_extracol1) end,
    case when epi_extracol2 is Null then '' else concat('|', epi_extracol2) end,
    case when epi_extracol3 is Null then '' else concat('|', epi_extracol3) end,
    /*case when epi_season is Null then '' else concat('|Staffel: ', cast(epi_season as char)) end,*/
    case when epi_season is Null then
      case when cnt_longdescription is Null then '' else
        case when locate('. Staffel, Folge', cnt_longdescription) = 0 then '' else
          concat('||Staffel: ', substring_index(cnt_longdescription, '.', 1)) end end
    else
      concat('|Staffel: ', cast(epi_season as char)) end,
    /*case when epi_part is Null then '' else concat('|Episode: ', cast(epi_part as char)) end,*/
    case when epi_part is Null then
      case when cnt_longdescription is Null then '' else
        case when locate('. Staffel, Folge', cnt_longdescription) = 0 then '' else
          concat('|Episode: ', substring_index(substring_index(cnt_longdescription, 'Folge ', -1), ':', 1)) end end
    else
      concat('|Episode: ', cast(epi_part as char)) end,
    case when epi_parts is Null then '' else concat('|Staffelfolgen: ', cast(epi_parts as char)) end,
    case when epi_number is Null then '' else concat('|Folge: ', cast(epi_number as char)) end,
    case when cnt_source <> sub_source then concat('||EPG: ',upper(replace(cnt_source, 'vdr', 'dvb')),'/',upper(sub_source)) else concat('||EPG: ', upper(replace(cnt_source, 'vdr', 'dvb'))) end
  )
)
,'|', '
') as description
from
 useevents;
