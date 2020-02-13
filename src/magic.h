/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/


/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_rose	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(      spell_demonfire		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_good	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(      spell_dispel_good       );
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_fireproof		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_floating_disc	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_heat_metal	);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_nexus		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_protection_evil	);
DECLARE_SPELL_FUN(	spell_protection_good	);
DECLARE_SPELL_FUN(	spell_ray_of_truth	);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);


//Upro

DECLARE_SPELL_FUN(	spell_fear				);
DECLARE_SPELL_FUN(  spell_knock				);
DECLARE_SPELL_FUN(  spell_confusion			);
DECLARE_SPELL_FUN(  spell_web				);
DECLARE_SPELL_FUN(  spell_silence			);
DECLARE_SPELL_FUN(  spell_scry				);
DECLARE_SPELL_FUN(  spell_detect_scrying	);
DECLARE_SPELL_FUN(  spell_displacement		);
DECLARE_SPELL_FUN(  spell_disrupt_undead	);
DECLARE_SPELL_FUN(  spell_enlarge_person	);
DECLARE_SPELL_FUN(  spell_etherealness		);
DECLARE_SPELL_FUN(  spell_feeblemind		);
DECLARE_SPELL_FUN(  spell_paralyzation		);
DECLARE_SPELL_FUN(  spell_detect_undead		);
DECLARE_SPELL_FUN(  spell_detect_traps		);
DECLARE_SPELL_FUN(	spell_shadow_breath		);
DECLARE_SPELL_FUN(	spell_energy_breath		);
DECLARE_SPELL_FUN(	spell_audible_glamor	);
DECLARE_SPELL_FUN(	spell_extension			);
DECLARE_SPELL_FUN(	spell_shadow_bolt		);
DECLARE_SPELL_FUN(	spell_mana_shield		);
DECLARE_SPELL_FUN(	spell_acid_arrow		);
DECLARE_SPELL_FUN(	spell_flame_arrow		);
DECLARE_SPELL_FUN(	spell_frost_arrow		);
DECLARE_SPELL_FUN(	spell_lay_hands			);
DECLARE_SPELL_FUN(	spell_blink				);
DECLARE_SPELL_FUN(  spell_summon_monster	);
DECLARE_SPELL_FUN(  spell_embalm			);


//Druid spells:
DECLARE_SPELL_FUN(	spell_resistance		);
DECLARE_SPELL_FUN(	spell_barkskin			);
DECLARE_SPELL_FUN(	spell_shillelagh		);
DECLARE_SPELL_FUN(	spell_stone_meld		);
DECLARE_SPELL_FUN(	spell_entangle			);
DECLARE_SPELL_FUN(	spell_detect_plants		);

DECLARE_SPELL_FUN(	spell_animal_growth		);
DECLARE_SPELL_FUN(	spell_sunbeam			);
DECLARE_SPELL_FUN(	spell_fire_seeds		);
DECLARE_SPELL_FUN(	spell_thorns			);
DECLARE_SPELL_FUN(	spell_changestaff		);
DECLARE_SPELL_FUN(	spell_water_breathing	);
DECLARE_SPELL_FUN(	spell_transmute_metal	);
DECLARE_SPELL_FUN(	spell_regeneration		);


//Bard spells
DECLARE_SPELL_FUN(spell_adagio);
DECLARE_SPELL_FUN(spell_vocal_shield);
DECLARE_SPELL_FUN(spell_vitalizing_verse);
DECLARE_SPELL_FUN(spell_hypnotic_verse);
DECLARE_SPELL_FUN(spell_nourishing_ballad);
DECLARE_SPELL_FUN(spell_motivation);
DECLARE_SPELL_FUN(spell_insidious_chord);
DECLARE_SPELL_FUN(spell_banshee_ballad);
DECLARE_SPELL_FUN(spell_calming_verse);
DECLARE_SPELL_FUN(spell_crescendo);
DECLARE_SPELL_FUN(spell_quicken_tempo);
DECLARE_SPELL_FUN(spell_orbitting_rhythm_lead);
DECLARE_SPELL_FUN(spell_suggestive_verse);
DECLARE_SPELL_FUN(spell_melody_masked_intent);
DECLARE_SPELL_FUN(spell_forgetful_lullaby);
DECLARE_SPELL_FUN(spell_hymn_fallen);
DECLARE_SPELL_FUN(spell_contradicting_melody);
DECLARE_SPELL_FUN(spell_cacophonic_shield);
DECLARE_SPELL_FUN(spell_dissonant_chord);
DECLARE_SPELL_FUN(spell_dirge_dischord);
DECLARE_SPELL_FUN(spell_harmonic_chorus);
DECLARE_SPELL_FUN(spell_improvisation);
DECLARE_SPELL_FUN(spell_infernal_threnody);
DECLARE_SPELL_FUN(spell_listening_jewel);
DECLARE_SPELL_FUN(spell_wail_doom);
DECLARE_SPELL_FUN(spell_resonating_echo);
DECLARE_SPELL_FUN(spell_selective_hearing);
DECLARE_SPELL_FUN(spell_sacred_euphony);
DECLARE_SPELL_FUN(spell_repulsive_din);
DECLARE_SPELL_FUN(spell_audible_intoxication);
DECLARE_SPELL_FUN(spell_diminishing_scale);
DECLARE_SPELL_FUN(spell_aggressive_staccato);
DECLARE_SPELL_FUN(spell_assisted_coda);
DECLARE_SPELL_FUN(spell_dynamic_diapason);
DECLARE_SPELL_FUN(spell_gnomish_tribute);
DECLARE_SPELL_FUN(spell_wrath_ancient);	
DECLARE_SPELL_FUN(spell_disintegrate);
DECLARE_SPELL_FUN(spell_blade_barrier);
DECLARE_SPELL_FUN(spell_aid);

DECLARE_SPELL_FUN(spell_globe_invulnerability);
DECLARE_SPELL_FUN(spell_gaseous_form);
DECLARE_SPELL_FUN(spell_ghoul_touch);
DECLARE_SPELL_FUN(spell_forbiddance);
DECLARE_SPELL_FUN(spell_explosive_runes);
DECLARE_SPELL_FUN(spell_fire_storm);
DECLARE_SPELL_FUN(spell_gust_of_wind);
DECLARE_SPELL_FUN(spell_ice_storm);
DECLARE_SPELL_FUN(spell_meteor_swarm);
DECLARE_SPELL_FUN(spell_nightmare);
DECLARE_SPELL_FUN(spell_create_fire);
DECLARE_SPELL_FUN(spell_resist_scrying);
DECLARE_SPELL_FUN(spell_retribution);
DECLARE_SPELL_FUN(spell_astral_spell);


DECLARE_SPELL_FUN(spell_strength);
DECLARE_SPELL_FUN(spell_intelligence);
DECLARE_SPELL_FUN(spell_wisdom);
DECLARE_SPELL_FUN(spell_dexterity);
DECLARE_SPELL_FUN(spell_constitution);
DECLARE_SPELL_FUN(spell_charisma);

DECLARE_SPELL_FUN(spell_animate_object);
DECLARE_SPELL_FUN(spell_banshee_wail);
DECLARE_SPELL_FUN(spell_copy);
DECLARE_SPELL_FUN(spell_chromatic_orb);
DECLARE_SPELL_FUN(spell_corpse_visage);
DECLARE_SPELL_FUN(spell_death_recall);
DECLARE_SPELL_FUN(spell_negative_plane_protection);
DECLARE_SPELL_FUN(spell_obscure_object);
DECLARE_SPELL_FUN(spell_timestop);
DECLARE_SPELL_FUN(spell_planar_door);
DECLARE_SPELL_FUN(spell_free_action);
DECLARE_SPELL_FUN(spell_cone_cold);
DECLARE_SPELL_FUN(spell_cone_shadow);
DECLARE_SPELL_FUN(spell_cone_sound);
DECLARE_SPELL_FUN(spell_cone_silence);
DECLARE_SPELL_FUN(spell_pass_without_trace);
DECLARE_SPELL_FUN(spell_searing_light);
DECLARE_SPELL_FUN(spell_sunburst);



DECLARE_SPELL_FUN(spell_read_object);
DECLARE_SPELL_FUN(spell_absorb_disease);
DECLARE_SPELL_FUN(spell_adrenaline_control);
DECLARE_SPELL_FUN(spell_agitation);
DECLARE_SPELL_FUN(spell_animate_shadow);
DECLARE_SPELL_FUN(spell_appraise);
DECLARE_SPELL_FUN(spell_aura_sight);
DECLARE_SPELL_FUN(spell_aversion);
DECLARE_SPELL_FUN(spell_awe);
DECLARE_SPELL_FUN(spell_ballistic_attack);
DECLARE_SPELL_FUN(spell_banishment);
DECLARE_SPELL_FUN(spell_biofeedback);
DECLARE_SPELL_FUN(spell_body_weaponry);
DECLARE_SPELL_FUN(spell_body_equilibrium);
DECLARE_SPELL_FUN(spell_chemical_simulation);
DECLARE_SPELL_FUN(spell_clairaudience);
DECLARE_SPELL_FUN(spell_clairvoyance);
DECLARE_SPELL_FUN(spell_clear_thoughts);
DECLARE_SPELL_FUN(spell_contact);
DECLARE_SPELL_FUN(spell_fighting_mind);
DECLARE_SPELL_FUN(spell_control_flame);
DECLARE_SPELL_FUN(spell_daydream);
DECLARE_SPELL_FUN(spell_death_field);
DECLARE_SPELL_FUN(spell_detonate);
DECLARE_SPELL_FUN(spell_domination);
DECLARE_SPELL_FUN(spell_dimensional_door);
DECLARE_SPELL_FUN(spell_ectoplasmic_form);
DECLARE_SPELL_FUN(spell_ego_whip);
DECLARE_SPELL_FUN(spell_ejection);
DECLARE_SPELL_FUN(spell_energy_containment);
DECLARE_SPELL_FUN(spell_enhanced_strength);
DECLARE_SPELL_FUN(spell_expansion);
DECLARE_SPELL_FUN(spell_flesh_armor);
DECLARE_SPELL_FUN(spell_immovability);
DECLARE_SPELL_FUN(spell_lethargy);
DECLARE_SPELL_FUN(spell_inertial_barrier);
DECLARE_SPELL_FUN(spell_intellect_fortress);
DECLARE_SPELL_FUN(spell_heightened_senses);
DECLARE_SPELL_FUN(spell_know_location);
DECLARE_SPELL_FUN(spell_lend_health);
DECLARE_SPELL_FUN(spell_levitate);
DECLARE_SPELL_FUN(spell_mental_barrier);
DECLARE_SPELL_FUN(spell_metabolic_reconfiguration);
DECLARE_SPELL_FUN(spell_mind_over_body);
DECLARE_SPELL_FUN(spell_mind_thrust);
DECLARE_SPELL_FUN(spell_mindwipe);
DECLARE_SPELL_FUN(spell_planar_gate);
DECLARE_SPELL_FUN(spell_probe);
DECLARE_SPELL_FUN(spell_psychic_drain);
DECLARE_SPELL_FUN(spell_reduction);
DECLARE_SPELL_FUN(spell_summon_creature);
DECLARE_SPELL_FUN(spell_holy_forge);

DECLARE_SPELL_FUN(spell_flaming_forge);
DECLARE_SPELL_FUN(spell_frost_forge);
DECLARE_SPELL_FUN(spell_lightning_forge);
DECLARE_SPELL_FUN(spell_vampiric_forge);

DECLARE_SPELL_FUN(spell_antimagic_shell);
DECLARE_SPELL_FUN(spell_earth_ward);
DECLARE_SPELL_FUN(spell_dimensional_mine);
DECLARE_SPELL_FUN(spell_shocking_web);
DECLARE_SPELL_FUN(spell_geyser);
DECLARE_SPELL_FUN(spell_shocking_runes);
DECLARE_SPELL_FUN(spell_animate_weapon);
DECLARE_SPELL_FUN(spell_hardstaff);
DECLARE_SPELL_FUN(spell_finger_death);
DECLARE_SPELL_FUN(spell_remove_paralysis);


DECLARE_SPELL_FUN(spell_alarm);
DECLARE_SPELL_FUN(spell_animate_dead);
DECLARE_SPELL_FUN(spell_raise_dead);
DECLARE_SPELL_FUN(spell_wither);
DECLARE_SPELL_FUN(spell_blasphemy);
DECLARE_SPELL_FUN(spell_bless_water);
DECLARE_SPELL_FUN(spell_deafness);
DECLARE_SPELL_FUN(spell_disenchant);
DECLARE_SPELL_FUN(spell_command_undead);
DECLARE_SPELL_FUN(spell_commune_nature);
DECLARE_SPELL_FUN(spell_comprehend_languages);
DECLARE_SPELL_FUN(spell_darkness);
DECLARE_SPELL_FUN(spell_dimensional_anchor);
DECLARE_SPELL_FUN(spell_elemental_protection);
DECLARE_SPELL_FUN(spell_erase);
DECLARE_SPELL_FUN(spell_feather_fall);
DECLARE_SPELL_FUN(spell_rehydrate);
DECLARE_SPELL_FUN(spell_frostbite);
DECLARE_SPELL_FUN(spell_bleeding_touch);
DECLARE_SPELL_FUN(spell_spectral_hand);
DECLARE_SPELL_FUN(spell_monsoon);
DECLARE_SPELL_FUN(spell_frostbolt);

DECLARE_SPELL_FUN(spell_summon_air);
DECLARE_SPELL_FUN(spell_summon_water);
DECLARE_SPELL_FUN(spell_summon_fire);
DECLARE_SPELL_FUN(spell_summon_earth);


DECLARE_SPELL_FUN(spell_manavert);
DECLARE_SPELL_FUN(spell_transfer_life);
DECLARE_SPELL_FUN(spell_reduce_person);