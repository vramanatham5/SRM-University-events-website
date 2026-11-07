
const SUPABASE_URL = "https://mmiduvoyvqpnymkfehqo.supabase.co";
const SUPABASE_KEY = "sb_publishable_lX6yaIkBUybaJGy_gtBogg_Odb1d0It";

const supabaseClient = window.supabase.createClient(
    SUPABASE_URL,
    SUPABASE_KEY
);

const loginForm = document.getElementById("admin-login-form");
const message = document.getElementById("message");

loginForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    message.textContent = "Checking login...";

    const email = document.getElementById("email").value.trim();
    const password = document.getElementById("password").value;

    try {
        // Step 1: Sign in using Supabase Auth
        const { data, error } =
            await supabaseClient.auth.signInWithPassword({
                email: email,
                password: password
            });

        if (error) throw error;

        // Step 2: Check the user's role
        const { data: profile, error: profileError } =
            await supabaseClient
                .from("portal_profiles")
                .select("role")
                .eq("id", data.user.id)
                .single();

        if (profileError) throw profileError;

        // Step 3: Allow only administrators
        if (profile.role !== "admin") {
            await supabaseClient.auth.signOut();
            throw new Error("Access denied. Admin account required.");
        }

        // Step 4: Open admin dashboard
        window.location.href = "dashboard.html";

    } catch (error) {
        message.textContent = error.message;
    }
});
