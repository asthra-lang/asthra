#include "test_immutable_by_default_common.h"

// =============================================================================
// REAL-WORLD SCENARIO TESTS
// =============================================================================

bool test_real_world_game_engine_scenario(void) {
    printf("Testing real-world game engine scenario...\n");
    
    const char *game_engine_code = 
        "struct GameState {\n"
        "    player_position: Point3D,\n"
        "    enemy_positions: [Point3D],\n"
        "    score: i32,\n"
        "    level: i32,\n"
        "    time_remaining: f32\n"
        "}\n"
        "\n"
        "struct Point3D {\n"
        "    x: f32,\n"
        "    y: f32,\n"
        "    z: f32\n"
        "}\n"
        "\n"
        "pub fn update_game_frame(current_state: GameState, delta_time: f32) -> GameState {\n"
        "    // Update player position based on input\n"
        "    let player_input: Point3D = get_player_input();\n"
        "    let new_player_pos: Point3D = Point3D {\n"
        "        x: current_state.player_position.x + player_input.x * delta_time,\n"
        "        y: current_state.player_position.y + player_input.y * delta_time,\n"
        "        z: current_state.player_position.z + player_input.z * delta_time\n"
        "    };\n"
        "    \n"
        "    // Update enemy positions\n"
        "    let updated_enemies: [Point3D] = update_enemy_positions(\n"
        "        current_state.enemy_positions, \n"
        "        new_player_pos, \n"
        "        delta_time\n"
        "    );\n"
        "    \n"
        "    // Check for collisions and update score\n"
        "    let collision_result: CollisionResult = check_collisions(new_player_pos, updated_enemies);\n"
        "    let new_score: i32 = current_state.score + collision_result.score_delta;\n"
        "    \n"
        "    // Update time\n"
        "    let new_time: f32 = current_state.time_remaining - delta_time;\n"
        "    \n"
        "    // Create new game state\n"
        "    let next_state: GameState = GameState {\n"
        "        player_position: new_player_pos,\n"
        "        enemy_positions: updated_enemies,\n"
        "        score: new_score,\n"
        "        level: current_state.level,\n"
        "        time_remaining: new_time\n"
        "    };\n"
        "    \n"
        "    return next_state;\n"
        "}";
    
    bool result = compile_and_validate_asthra_code(game_engine_code, "GameState next_state");
    
    if (!result) {
        printf("  ❌ Game engine scenario test failed\n");
        return false;
    }
    
    printf("  ✅ Real-world game engine scenario test passed\n");
    return true;
}

bool test_real_world_web_server_scenario(void) {
    printf("Testing real-world web server scenario...\n");
    
    const char *web_server_code = 
        "struct HttpRequest {\n"
        "    method: string,\n"
        "    path: string,\n"
        "    headers: [string],\n"
        "    body: string\n"
        "}\n"
        "\n"
        "struct HttpResponse {\n"
        "    status_code: i32,\n"
        "    headers: [string],\n"
        "    body: string\n"
        "}\n"
        "\n"
        "pub fn handle_api_request(request: HttpRequest) -> HttpResponse {\n"
        "    // Validate request method\n"
        "    let is_post: bool = (request.method == \"POST\");\n"
        "    let is_api_path: bool = request.path.starts_with(\"/api/\");\n"
        "    \n"
        "    if !is_post || !is_api_path {\n"
        "        let error_response: HttpResponse = HttpResponse {\n"
        "            status_code: 400,\n"
        "            headers: [\"Content-Type: application/json\"],\n"
        "            body: \"{\\\"error\\\": \\\"Invalid request\\\"}\"\n"
        "        };\n"
        "        return error_response;\n"
        "    }\n"
        "    \n"
        "    // Parse request body\n"
        "    let parsed_data: JsonValue = parse_json(request.body);\n"
        "    \n"
        "    // Process the data (immutable transformations)\n"
        "    let validated_data: JsonValue = validate_input(parsed_data);\n"
        "    let processed_data: JsonValue = apply_business_logic(validated_data);\n"
        "    let response_body: string = serialize_response(processed_data);\n"
        "    \n"
        "    let response: HttpResponse = HttpResponse {\n"
        "        status_code: 200,\n"
        "        headers: [\"Content-Type: application/json\"],\n"
        "        body: response_body\n"
        "    };\n"
        "    \n"
        "    return response;\n"
        "}";
    
    bool result = compile_and_validate_asthra_code(web_server_code, "HttpResponse");
    
    if (!result) {
        printf("  ❌ Web server scenario test failed\n");
        return false;
    }
    
    printf("  ✅ Real-world web server scenario test passed\n");
    return true;
} 
