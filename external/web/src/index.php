<!doctype html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="./css/style.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.2/css/all.min.css" integrity="sha512-z3gLpd7yknf1YoNbCzqRKc4qyor8gaKU1qmn+CShxbuBusANI9QpRohGBreCFkKxLhei6S9CQXFEbbKuqLg0DA==" crossorigin="anonymous" referrerpolicy="no-referrer" />
    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
</head>
<body class="bg-gray-100 dark:bg-gray-900">
    <section class="m-5 flex flex-col gap-4">
        <div class="flex gap-4">
            <div class="flex gap-2 flex-wrap w-full">
                <select id="references-select" onChange="reloadPlayers()" class="max-w-sm bg-gray-50 border border-gray-300 text-gray-900 rounded-lg  block w-full p-2.5 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white"></select>
                <select id="maps-select" onChange="reloadPlayers()" class="max-w-sm bg-gray-50 border border-gray-300 text-gray-900 rounded-lg  block w-full p-2.5 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white"></select>
            </div>
            <div class="cursor-pointer ml-auto text-3xl font-extrabold tracking-tight text-gray-900 dark:text-white">
                <i id="theme-icon" onclick="changeTheme()"></i>
            </div>
        </div>
        <div class="overflow-auto rounded-lg shadow-md dark:bg-gray-800 dark:text-white">
            <table class="pt-5 w-full border-collapse dark:bg-gray-800 text-left text-sm text-gray-500 dark:text-white">
                <thead class="bg-gray-50 dark:bg-gray-700 dark:text-gray-400">
                    <tr>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">
                            <i class="fa-solid fa-ranking-star"></i>
                        </th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Name</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Points</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Kills</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Assist</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Death</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Bomb</th>
                    </tr>
                </thead>
                <tbody id="players-table" class="flex-1 sm:flex-none">

                </tbody>
            </table>
        </div>

        <div class="flex">
            <button onclick="previousPage()" id="previousPage" class="flex items-center justify-center px-3 h-8 text-sm font-medium text-gray-500 bg-white border border-gray-300 rounded-lg rounded-r-none disabled:opacity-50 disabled:pointer-events-none hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white">
                Prev
            </button>
            <span id="currentPage" class="flex items-center justify-center px-3 h-8 text-sm font-medium text-gray-500 bg-white border border-x-0 border-gray-300 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400"></span>
            <button onclick="nextPage()" id="nextPage" class="flex items-center justify-center px-3 h-8 text-sm font-medium text-gray-500 bg-white border border-gray-300 rounded-lg rounded-l-none disabled:opacity-50 disabled:pointer-events-none hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white">
                Next
            </button>
        </div>
    </section>

    <footer class="pt-5 flex gap-1 justify-center text-gray-400">
        <div class="flex gap-1 items-center">
            <a href="https://github.com/Salvatore-Als/cs2-rank" class="text-gray-400 hover:text-gray-600 transition">
                <i class="fab fa-github"></i>
            </a>
            <span>CS2 Rank by</span>
            <a href="https://twitter.com/kriax_" class="text-blue-400 hover:text-blue-600 transition">Kriax</a>
        </div>
        <span>-</span>
        <div class="flex gap-1">
            <span>Sponsored by</span>
            <a href="https://www.verygames.net/en" class="text-blue-400 hover:text-blue-600 transition">VeryGames</a>
        </div>
    </footer>

    <script src="./js/script.js"></script>
</body>
</html>
